#ifndef DISTRIBUTE_H
#define DISTRIBUTE_H

#include <mpi.h>

void distribute_by_median(double *local_points, int num_points, int points_dim,
                          int world_size, int process_rank, MPI_Comm comm);

#endif