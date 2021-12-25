#include "test.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

int test_validity(double *local_points, int num_local_points, int points_dim,
                  double *pivot, int process_rank, int cluster_size) {
  // Find distances from pivot for every point in current process
  double *local_distances = (double *)malloc(num_local_points * sizeof(double));

  double *point;
  for (int i = 0; i < num_local_points; i++) {
    point = get_nth_point(local_points, num_local_points, points_dim, i);
    local_distances[i] = euclidean_dist(point, pivot, points_dim);
    free(point);
  }

  // Find the min and max distance from pivot for each process
  double local_minmax[2];
  local_minmax[0] = find_min(local_distances, num_local_points);
  local_minmax[1] = find_max(local_distances, num_local_points);

  // Master process gathers the minmax arrays
  double *minmax;
  if (process_rank == 0)
    minmax = (double *)malloc(2 * cluster_size * sizeof(double));

  MPI_Gather(&local_minmax, 2, MPI_DOUBLE, minmax, 2, MPI_DOUBLE, 0,
             MPI_COMM_WORLD);

  free(local_distances);

  // Max distance for process n must be smaller than min distance of process n+1
  if (process_rank == 0) {
    for (int i = 0; i < cluster_size - 1; i++) {
      if (minmax[2 * i + 1] > minmax[2 * (i + 1)]) {
        free(minmax);
        return 0;
      }
    }

    free(minmax);
    return 1;
  } else {
    free(minmax);
    return -1;
  }
}