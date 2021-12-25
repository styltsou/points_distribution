#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "distribute.h"
#include "read.h"
#include "test.h"
#include "utils.h"

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int cluster_size;
  MPI_Comm_size(MPI_COMM_WORLD, &cluster_size);

  int process_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

  int num_points;
  if (process_rank == 0) num_points = atoi(argv[1]);

  // Get the number of points in the file and the points' dimensions
  int num_points_in_file;
  int points_dim;
  get_file_info(&num_points_in_file, &points_dim);

  if (process_rank == 0) {
    // Check if the number points specified are more than the file contains
    if (num_points > num_points_in_file) {
      printf("[!] The file does not contain that many points\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Check if the number points is a power of 2
    if (!is_power_of_two(num_points)) {
      fprintf(stderr, "[!] The total number of points must be a power of 2\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  MPI_Bcast(&num_points, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Each process reads a number points from the file
  int num_local_points = num_points / cluster_size;

  double *local_points =
      (double *)malloc(num_local_points * points_dim * sizeof(double));

  read_points(local_points, num_local_points * points_dim);

  double wall_time = MPI_Wtime();
  /*
    Master (process 0) randomly selects one of its points as pivot and
    broadcasts it to all the other processess
  */
  double *pivot = (double *)malloc(points_dim * sizeof(double));

  if (process_rank == 0)
    pivot = get_nth_point(local_points, num_local_points, points_dim,
                          rand_int(0, num_local_points));

  MPI_Bcast(pivot, points_dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  /*
    Every process computes the distance of each local
    point from pivot
  */
  double *local_distances = (double *)malloc(num_local_points * sizeof(double));

  for (int i = 0; i < num_local_points; i++)
    local_distances[i] =
        euclidean_dist(&local_points[i * points_dim], pivot, points_dim);

  /*
    Now that we know both the points themselves and their distances, we will
    merge them into one array. The structure of the array will be the
    following: [local_point_0, local_distance_0, local_point_1,
    local_distance_1, ...]
  */
  double *aug_local_points = (double *)malloc(
      (num_local_points * points_dim + num_local_points) * sizeof(double));

  // Merge the points and their distances
  for (int i = 0; i < num_local_points; i++) {
    aug_local_points[i * (points_dim + 1) + points_dim] = local_distances[i];
    for (int j = 0; j < points_dim; j++) {
      aug_local_points[i * (points_dim + 1) + j] =
          local_points[i * points_dim + j];
    }
  }

  distribute_by_median(aug_local_points, num_points, points_dim + 1,
                       cluster_size, process_rank, MPI_COMM_WORLD);

  /*
    After the points are redistributed, remove the extra dimension (distance)
    and store the points back to local_points array
  */
  for (int i = 0; i < num_local_points; i++) {
    for (int j = 0; j < points_dim; j++)
      local_points[i * points_dim + j] =
          aug_local_points[i * (points_dim + 1) + j];
  }

  free(aug_local_points);

  wall_time = MPI_Wtime() - wall_time;

  int res = test_validity(local_points, num_local_points, points_dim, pivot,
                          process_rank, cluster_size);
  if (process_rank == 0)
    if (!res) fprintf(stderr, "ERROR: test failed!\n");

  // Get max wall time
  double max_wall_time;

  MPI_Reduce(&wall_time, &max_wall_time, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  if (process_rank == 0) printf("%lf\n", max_wall_time);

  MPI_Finalize();
}