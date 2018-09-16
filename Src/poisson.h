#pragma once

#include <functional>

namespace poisson {

bool poisson1d(int numIter, float min, float max, int n, float   *data, std::function<float(float)>&& f);
bool poisson2d(int numIter, float min, float max, int n, float  **data, std::function<float(float, float)>&& f);
bool poisson3d(int numIter, float min, float max, int n, float ***data, std::function<float(float, float, float)>&& f);

}