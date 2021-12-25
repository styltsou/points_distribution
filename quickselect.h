#ifndef QUICKSELECT_H
#define QUICKSELECT_H

void swap(double *a, double *b);

int partition(double *arr, int l, int r);

double quickselect(double *arr, int l, int r, int k);

#endif