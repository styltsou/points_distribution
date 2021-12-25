#include "distribute.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
void distribute_by_median(double *local_points, int num_points, int points_dim,
                          int comm_size, int process_rank, MPI_Comm comm) {
  // Number of points in each process
  int num_local_points = num_points / comm_size;

  int pr;
  MPI_Comm_rank(MPI_COMM_WORLD, &pr);

  // For every local point store its distance from pivot
  double *local_distances = (double *)malloc(num_local_points * sizeof(double));
  for (int i = 0; i < num_local_points; i++)
    local_distances[i] = local_points[i * points_dim + points_dim - 1];

  // Master process gathers all the distances
  double *distances;
  if (process_rank == 0)
    distances = (double *)malloc(num_points * sizeof(double));

  MPI_Gather(local_distances, num_local_points, MPI_DOUBLE, distances,
             num_local_points, MPI_DOUBLE, 0, comm);

  // Master process calculates the median distance anf broadcasts it
  double median_dist;
  if (process_rank == 0) median_dist = get_median(distances, num_points);

  MPI_Bcast(&median_dist, 1, MPI_DOUBLE, 0, comm);
  // Positions of local_out_points in the local_points array
  int *local_out_points_pos = (int *)malloc(num_local_points * sizeof(int));

  /*
    Each process finds which points need to be exchanged depending on the
    process group they belong. The first half processes (left group) need to
    have points with distance smaller than median.
  */
  int num_local_out_points = 0;
  if (process_rank < comm_size / 2) {
    // Process belongs to left group
    for (int i = 0; i < num_local_points; i++) {
      if (local_distances[i] > median_dist) {
        local_out_points_pos[num_local_out_points] = i;
        num_local_out_points++;
      }
    }
  } else {
    // Process belongs to right group
    for (int i = 0; i < num_local_points; i++) {
      if (local_distances[i] < median_dist) {
        local_out_points_pos[num_local_out_points] = i;
        num_local_out_points++;
      }
    }
  }

  // Reallocate memory based on the actual number of local outgoing points
  local_out_points_pos =
      (int *)realloc(local_out_points_pos, num_local_out_points * sizeof(int));

  // Store the points that need to be exchanged from the process
  double *local_out_points =
      (double *)malloc(num_local_out_points * points_dim * sizeof(double));

  double *point;
  for (int i = 0; i < num_local_out_points; i++) {
    point = get_nth_point(local_points, num_local_points, points_dim,
                          local_out_points_pos[i]);
    insert_nth_point(local_out_points, num_local_out_points, points_dim, i,
                     point);
    free(point);
  }

  /*
    if the communicator contains 2 processes, then the point exchange is done
    using point-to-point communications
  */
  if (comm_size == 2) {
    // Allocate space to store the incoming local points
    double *local_in_points =
        (double *)malloc(num_local_out_points * points_dim * sizeof(double));

    // Process 0 sends first and then receives
    // Process 1 receives and then sends
    if (process_rank == 0) {
      MPI_Send(local_out_points, num_local_out_points * points_dim, MPI_DOUBLE,
               1, 0, comm);
      MPI_Recv(local_in_points, num_local_out_points * points_dim, MPI_DOUBLE,
               1, 1, comm, MPI_STATUS_IGNORE);
    } else {
      MPI_Recv(local_in_points, num_local_out_points * points_dim, MPI_DOUBLE,
               0, 0, comm, MPI_STATUS_IGNORE);
      MPI_Send(local_out_points, num_local_out_points * points_dim, MPI_DOUBLE,
               0, 1, comm);
    }

    //  Insert the local incoming points in the correct position in local_points
    for (int i = 0; i < num_local_out_points; i++) {
      point =
          get_nth_point(local_in_points, num_local_out_points, points_dim, i);
      insert_nth_point(local_points, num_local_points, points_dim,
                       local_out_points_pos[i], point);
      free(point);
    }
  } else {
    /*
      Split current communicator into 2 other communicators.
      Processes 0...(n/2)-1 get color 0, the rest get color 1
    */
    MPI_Comm split_comm;
    MPI_Comm_split(comm, process_rank / (comm_size / 2), process_rank,
                   &split_comm);

    int split_comm_proc_rank;
    MPI_Comm_rank(split_comm, &split_comm_proc_rank);
    /*
      Find the number of points that each process of the group
      needs to exchange and store it in an array
    */
    int *out_points_counts;
    if (split_comm_proc_rank == 0)
      out_points_counts = (int *)malloc(comm_size / 2 * sizeof(int));

    MPI_Gather(&num_local_out_points, 1, MPI_INT, out_points_counts, 1, MPI_INT,
               0, split_comm);
    /*
      Calculate the data needed to gather all the local out_points of the group
      at the group's master process
    */
    double *out_points;
    int num_out_points;
    int *out_points_dspls;

    if (split_comm_proc_rank == 0) {
      // Find the total number of points to be gathered
      num_out_points = 0;
      for (int i = 0; i < comm_size / 2; i++)
        num_out_points += out_points_counts[i];

      // Calculate the displacements
      for (int i = 0; i < comm_size / 2; i++)
        out_points_counts[i] *= points_dim;

      out_points_dspls = (int *)calloc(comm_size / 2, sizeof(int));

      for (int i = 1; i < comm_size / 2; i++)
        out_points_dspls[i] =
            out_points_counts[i - 1] + out_points_dspls[i - 1];

      // Allocate receive buffer for gather communication
      out_points =
          (double *)malloc(num_out_points * points_dim * sizeof(double));
    }

    MPI_Gatherv(local_out_points, num_local_out_points * points_dim, MPI_DOUBLE,
                out_points, out_points_counts, out_points_dspls, MPI_DOUBLE, 0,
                split_comm);

    /*
      Exchange out_points between the 2 groups
      Master process of left group will send first and then receive
      Master process of right group will receive and then send
    */
    double *in_points;
    if (process_rank == 0) {
      in_points =
          (double *)malloc(num_out_points * points_dim * sizeof(double));
      MPI_Send(out_points, num_out_points * points_dim, MPI_DOUBLE,
               comm_size / 2, 0, comm);
      MPI_Recv(in_points, num_out_points * points_dim, MPI_DOUBLE,
               comm_size / 2, 1, comm, MPI_STATUS_IGNORE);
    } else if (process_rank == comm_size / 2) {
      in_points =
          (double *)malloc(num_out_points * points_dim * sizeof(double));
      MPI_Recv(in_points, num_out_points * points_dim, MPI_DOUBLE, 0, 0, comm,
               MPI_STATUS_IGNORE);
      MPI_Send(out_points, num_out_points * points_dim, MPI_DOUBLE, 0, 1, comm);
    }

    /*
      Scatter the in_points in each process of the group based on every
      processe's capacity
    */
    double *local_in_points =
        (double *)malloc(num_local_out_points * points_dim * sizeof(double));

    MPI_Scatterv(in_points, out_points_counts, out_points_dspls, MPI_DOUBLE,
                 local_in_points, num_local_out_points * points_dim, MPI_DOUBLE,
                 0, split_comm);

    // Insert every incoming point into the right positions of local_points
    for (int i = 0; i < num_local_out_points; i++) {
      point =
          get_nth_point(local_in_points, num_local_out_points, points_dim, i);
      insert_nth_point(local_points, num_local_points, points_dim,
                       local_out_points_pos[i], point);
      free(point);
    }

    // Free the unneeded communicator (except MPI_COMM_WORLD)
    int res;
    MPI_Comm_compare(MPI_COMM_WORLD, comm, &res);
    if (res != MPI_IDENT) MPI_Comm_free(&comm);

    // Call the method recursively for the 2 groups of processes
    distribute_by_median(local_points, num_points / 2, points_dim,
                         comm_size / 2, split_comm_proc_rank, split_comm);
  }
}