#include "quickselect.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void swap(double *a, double *b) {
  double temp = *a;
  *a = *b;
  *b = temp;
}

int partition(double *arr, int l, int r) {
  double pivot = arr[r];
  int i = l;

  for (int j = l; j <= r - 1; j++) {
    if (arr[j] <= pivot) {
      swap(&arr[i], &arr[j]);
      i++;
    }
  }

  swap(&arr[i], &arr[r]);
  return i;
}

double quickselect(double *arr, int l, int r, int k) {
  if (k > 0 && k <= r - l + 1) {
    int idx = partition(arr, l, r);

    if (idx - l == k - 1) return arr[idx];

    if (idx - l > k - 1) return quickselect(arr, l, idx - 1, k);

    return quickselect(arr, idx + 1, r, k - idx + l - 1);
  }

  // If k out of bounds
  printf("[!] quickselect: index out of bounds [!]\n");
  return (double)INT_MAX;
}
