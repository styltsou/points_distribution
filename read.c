#include "read.h"

#include <math.h>
#include <mpi.h>
#include <stdlib.h>

/*
 * Each process should know how many points there are and what's there
 * dimension. It updates the NULL variables passed in as arguments.
 */
void get_file_info(int *num_of_points_in_file, int *dimension_of_point) {
  MPI_File file;
  MPI_File_open(MPI_COMM_WORLD, "./data/points", MPI_MODE_RDONLY, MPI_INFO_NULL,
                &file);

  unsigned char *buf = (unsigned char *)malloc(4 * sizeof(unsigned char));

  MPI_File_read_all(file, buf, 4, MPI_CHAR, MPI_STATUS_IGNORE);
  int num_of_dimensions = (int)buf[3];

  MPI_File_read_all(file, buf, 4, MPI_CHAR, MPI_STATUS_IGNORE);
  *num_of_points_in_file = convert_hex_to_int(buf);

  *dimension_of_point = 1;
  for (int i = 0; i < num_of_dimensions - 1; i++) {
    MPI_File_read_all(file, buf, 4, MPI_CHAR, MPI_STATUS_IGNORE);
    *dimension_of_point *= convert_hex_to_int(buf);
  }
}

/*
 * Each process should get the raw data of the points.
 * It updates the local_points array passed in as argument.
 */
void read_points(double *local_points, int total_proc_size) {
  MPI_File file;
  MPI_File_open(MPI_COMM_WORLD, "./data/points", MPI_MODE_RDONLY, MPI_INFO_NULL,
                &file);

  MPI_Offset offset = 16;
  MPI_File_seek_shared(file, offset, MPI_SEEK_SET);

  unsigned char *buf =
      (unsigned char *)malloc(total_proc_size * sizeof(unsigned char));
  MPI_File_read_ordered(file, buf, total_proc_size, MPI_CHAR,
                        MPI_STATUS_IGNORE);

  for (int i = 0; i < total_proc_size; i++)
    local_points[i] = ((double)buf[i]) / 255;
}

/* Converts 4 byte hex to integer. */
int convert_hex_to_int(unsigned char *arr) {
  int result = 0;
  for (int i = 0, power = 6; i < 4; i++, power -= 2)
    result += (int)arr[i] * pow(16.0, (double)power);
  return result;
}