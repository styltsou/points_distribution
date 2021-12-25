#ifndef TEST_H
#define TEST_H

/***********************************************************************************
  * test_validity
  * description:
  * This method asserts the correctness of the points distribution algorithm.
  * The method tests the points for all the processes that belong to the
  MPI_COMM_WORLD communicator.
  * parameters:
  *   local_points - Array that contains the points stored in a process
  *   num_local_points - Number of points that each process has
  *   points_dim - Dimensions of every point
  *   pivot - Point that was selected as pivot
  *   process_rank - Rank of current process in MPI_COMM_WORLD
  * returns: 1 if the test succeeds and 0 if the test fails
  NOTE: The output is significant only at process 0
  *********************************************************************************/
int test_validity(double *local_points, int num_local_points, int points_dim,
                  double *pivot, int process_rank, int cluster_size);

#endif