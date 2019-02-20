#pragma once

#include <cstdlib>
#include <cmath>

#include "lpVec3.h"

inline float RAND()
{
    return ((float)rand() / RAND_MAX);
}

inline float RAND_RANGE(float a, float b)
{
    return a + (b - a) * RAND();
}

inline float deg2rad(float deg)
{
    return M_PI / 180.0f * deg;
}

inline float rad2deg(float rad)
{
    return 180.0f / M_PI * rad;
}

struct float2
{
    float x, y;

    float2(float xp = 0.0f, float yp = 0.0f) : x(xp), y(yp) {}
};

float integrate_rect(float a, float b, int n, float(*f)(float));
float integrate_trap(float a, float b, int n, float(*f)(float));

bool poisson1d(int numIter, float min, float max, int n, float   *data, float(*f)(float));
bool poisson2d(int numIter, float min, float max, int n, float  **data, float(*f)(float, float));
bool poisson3d(int numIter, float min, float max, int n, float ***data, float(*f)(float, float, float));

// Случайное число с нормальным распределением
float RandomStandardDistribution();

inline lpVec3 SphericalToCartesian(float r, float phi, float theta)
{
    return { r * std::sinf(theta) * std::cosf(phi), r * std::sinf(theta) * std::sinf(phi), r * std::cosf(theta) };
}

inline lpVec3 SphericalToCartesian(const lpVec3& spherical)
{
    return SphericalToCartesian(spherical.m_x, spherical.m_y, spherical.m_z);
}

inline lpVec3 CylindricalToCartesian(float r, float phi, float z)
{
    return { r * std::cosf(phi), r * std::sinf(phi), z };
}

inline lpVec3 CylindricalToCartesian(const lpVec3& cylindrical)
{
    return CylindricalToCartesian(cylindrical.m_x, cylindrical.m_y, cylindrical.m_z);
}

inline lpVec3 RandomUniformSpherical(float rmin, float rmax)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * float(M_PI) * RAND() - float(M_PI), 2.0f * float(M_PI) * RAND() };
}

inline lpVec3 RandomUniformCylindrical(float rmin, float rmax, float height)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * float(M_PI) * RAND(), RAND_RANGE(-0.5f * height, 0.5f * height) };
}