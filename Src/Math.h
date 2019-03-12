#pragma once

#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <vector>

#include "float3.h"

#define PI (static_cast<float>(3.14159265358979323846))

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
    return PI / 180.0f * deg;
}

inline float rad2deg(float rad)
{
    return 180.0f / PI * rad;
}

inline float lerp(float a, float b, float t)
{
    return (1.0f - t) * a + t * b;
}

struct float2
{
    float x, y;

    float2(float xp = 0.0f, float yp = 0.0f) : x(xp), y(yp) {}
};

inline float2 operator+(const float2& lhs, const float2& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline float2 operator-(const float2& lhs, const float2& rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

float integrate_rect(float a, float b, int n, float(*f)(float));
float integrate_trap(float a, float b, int n, float(*f)(float));

void Poisson1(uint32_t numIter, float min, float max, int n, float *data, const std::vector<float>& rightPart);

bool poisson1d(int numIter, float min, float max, int n, float   *data, float(*f)(float));
bool poisson2d(int numIter, float min, float max, int n, float  **data, float(*f)(float, float));
bool poisson3d(int numIter, float min, float max, int n, float ***data, float(*f)(float, float, float));

// Случайное число с нормальным распределением
float RandomStandardDistribution();

inline float3 SphericalToCartesian(float r, float phi, float theta)
{
    return { r * std::sinf(theta) * std::cosf(phi), r * std::sinf(theta) * std::sinf(phi), r * std::cosf(theta) };
}

inline float3 SphericalToCartesian(const float3& spherical)
{
    return SphericalToCartesian(spherical.m_x, spherical.m_y, spherical.m_z);
}

inline float3 CylindricalToCartesian(float r, float phi, float z)
{
    return { r * std::cosf(phi), r * std::sinf(phi), z };
}

inline float3 CylindricalToCartesian(const float3& cylindrical)
{
    return CylindricalToCartesian(cylindrical.m_x, cylindrical.m_y, cylindrical.m_z);
}

inline float3 RandomUniformSpherical(float rmin, float rmax)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * PI * RAND() /*- PI*/, 2.0f * PI * RAND() };
}

inline float3 RandomUniformCylindrical(float rmin, float rmax, float height)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * PI * RAND(), RAND_RANGE(-0.5f * height, 0.5f * height) };
}

inline float3 GravityAcceleration(const float3& point, float mass, float soft, float length = -1.0f)
{
    float3 acceleration = point;
    float distance = length > 0.0f ? (length + soft) : acceleration.norm() + soft;
    acceleration = acceleration * (mass / (distance * distance * distance));
    return acceleration;
}

/** Radial velocity about body with certain mass at distance r. */
inline float RadialVelocity(float mass, float r)
{
    return std::sqrtf(mass / r);
}

/** Radial velocity of body with mass in force field at distance r. */
inline float RadialVelocity(float force, float mass, float r)
{
    return std::sqrtf(force * r / mass);
}

inline float PseudoIsothermal(float r, float rho0, float radius)
{
    return rho0 / (1.0f + (r / radius) * (r / radius));
}

inline float PlummerDensity(float r, float mass, float radius)
{
    return (3.0f * mass / (4.0f * PI * radius * radius * radius)) * 
        (1.0f / std::sqrtf(std::pow((1.0f + (r * r) / (radius * radius)), 5.0f)));
}

inline float PlummerPotential(float r, float mass, float radius)
{
    return -mass / (std::sqrtf(r * r + radius * radius));
}

template <typename Distribution>
inline float SampleDistribution(float xmin, float xmax, float maxDistributionValue, Distribution distribution)
{
    float x = 0.0f, y = 0.0f;
    while (true)
    {
        x = RAND_RANGE(xmin, xmax);
        y = RAND_RANGE(0.0f, maxDistributionValue);
        if (y < distribution(x))
        {
            break;
        }
    }
    return x;
}

inline void IntegrateMotionEquation(float time, float3& position, float3& velocity, const float3& acceleration, const float3& force, float inverseMass)
{
    // Euler-Cromer
    float3 a = acceleration + force * inverseMass;
    velocity += acceleration * time;
    position += velocity * time;
}