#pragma once


bool poisson1d(int numIter, float min, float max, int n, float   *data, float (*f)(float));
bool poisson2d(int numIter, float min, float max, int n, float  **data, float (*f)(float, float));
bool poisson3d(int numIter, float min, float max, int n, float ***data, float (*f)(float, float, float));