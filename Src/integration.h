#pragma once

#include <functional>

namespace integration {

float rect(float a, float b, int n, std::function<float(float)>&& f);
float trap(float a, float b, int n, std::function<float(float)>&& f);

}