#include "utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "quickselect.h"

void print_point(int process_rank, double *point, int dim, char msg[]) {
  printf("%d :: ( ", process_rank);
  for (int i = 0; i < dim; i++) printf("%0.3lf ", point[i]);
  printf(") :: %s\n", msg);
}

double *get_points(int len, int process_rank) {
  srand(time(NULL) + process_rank);
  double *arr = (double *)malloc(len * sizeof(double));
  for (int i = 0; i < len; i++) arr[i] = (double)rand() / RAND_MAX;
  return arr;
}

double *get_nth_point(double *points, int num_points, int dim, int n) {
  double *point = (double *)malloc(dim * sizeof(double));
  if (n > num_points - 1) {
    fprintf(stderr, "[!] get_nth_point() index out of bounds [!]\n");
    exit(1);
  }
  for (int i = 0; i < dim; i++) point[i] = points[n * dim + i];
  return point;
}

void insert_nth_point(double *points, int num_points, int dim, int n,
                      double *point) {
  if (n > num_points - 1) {
    fprintf(stderr, "[!] insert_nth_point() index out of bounds[!]\n");
    exit(1);
  }
  for (int i = 0; i < dim; i++) points[n * dim + i] = point[i];
}

/* Given 2 points, it returns the euclidean distance between them */
double euclidean_dist(double *point1, double *point2, int dim) {
  double dist = 0.0;
  for (int i = 0; i < dim; i++) dist += pow(point2[i] - point1[i], 2.0);
  return sqrt(dist);
}

/* Given an array, it returns the median of the values in it */
double get_median(double *arr, int size) {
  return (quickselect(arr, 0, size - 1, size / 2) +
          quickselect(arr, 0, size - 1, size / 2 + 1)) /
         2;
}

/* Given an array, it returns the smallest value in it */
double find_min(double *arr, int len) {
  double min = 1000.0;
  for (int i = 0; i < len; i++)
    if (arr[i] < min) min = arr[i];
  return min;
}

/* Given an array, it returns the largest value in it */
double find_max(double *arr, int len) {
  double max = -1000.0;
  for (int i = 0; i < len; i++)
    if (arr[i] > max) max = arr[i];
  return max;
}

/* Returns a random integers from lower_limit (inclusive) to upper_limit
 * (exclusive) */
int rand_int(int lower_limit, int upper_limit) {
  srand(time(NULL));
  return (int)((double)rand() / RAND_MAX * upper_limit);
}

/* Returns 1 if x is a power of 2, 0 otherwise */
int is_power_of_two(int x) { return x && (!(x & (x - 1))); }