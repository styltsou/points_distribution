#ifndef UTILS_H
#define UTILS_H

void print_point(int process_rank, double *point, int dim, char msg[]);

double *get_points(int len, int process_rank);

int hex_to_int(unsigned char *data);

int randint(int a, int b);

double *get_nth_point(double *points, int num_points, int dim, int n);

void insert_nth_point(double *points, int num_points, int dim, int n,
                      double *point);

double euclidean_dist(double *point1, double *point2, int dim);

double get_median(double *data, int size);

double find_min(double *arr, int len);

double find_max(double *arr, int len);

int rand_int(int lower_limit, int upper_limit);

int is_power_of_two(int x);

#endif