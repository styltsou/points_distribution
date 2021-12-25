#ifndef READ_H
#define READ_H

void get_file_info(int *num_of_points_in_file, int *dimension_of_point);

void read_points(double *local_points, int total_proc_size);

int convert_hex_to_int(unsigned char *arr);
#endif