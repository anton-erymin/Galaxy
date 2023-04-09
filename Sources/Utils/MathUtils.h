#pragma once

#include "Math/Math.h"

float integrate_rect(float a, float b, int n, float(*f)(float));
float integrate_trap(float a, float b, int n, float(*f)(float));

void Poisson1(uint32_t numIter, float min, float max, int n, float *data, const vector<float>& rightPart);

bool poisson1d(int numIter, float min, float max, int n, float   *data, float(*f)(float));
bool poisson2d(int numIter, float min, float max, int n, float  **data, float(*f)(float, float));
bool poisson3d(int numIter, float min, float max, int n, float ***data, float(*f)(float, float, float));

float RandomStandardDistribution();

inline float3 SphericalToCartesian(float r, float phi, float theta)
{
    return { r * sinf(theta) * cosf(phi), r * sinf(theta) * sinf(phi), r * cosf(theta) };
}

inline float3 SphericalToCartesian(const float3& spherical)
{
    return SphericalToCartesian(spherical.x, spherical.y, spherical.z);
}

inline float3 CylindricalToCartesian(float r, float phi, float z)
{
    return { r * cosf(phi), r * sinf(phi), z };
}

inline float3 CylindricalToCartesian(const float3& cylindrical)
{
    return CylindricalToCartesian(cylindrical.x, cylindrical.y, cylindrical.z);
}

inline float3 RandomUniformSpherical(float rmin, float rmax)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * PI * RAND_NORM /*- PI*/, 2.0f * PI * RAND_NORM };
}

inline float3 RandomUniformCylindrical(float rmin, float rmax, float height)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * PI * RAND_NORM, RAND_RANGE(-0.5f * height, 0.5f * height) };
}

inline float3 GravityAcceleration(const float3& l, float mass, float soft, float length = -1.0f)
{
    float3 acceleration = l;
    float distance_sq = length > 0.0f ? (length * length) : dot(l, l);
    float r = sqrtf(distance_sq + soft * soft);
    float denom = r * r * r;
    acceleration *= mass / denom;
    return acceleration;
}

/** Radial velocity about body with certain mass at distance r. */
inline float RadialVelocity(float mass, float r)
{
    return sqrtf(mass / r);
}

/** Radial velocity of body with mass in force field at distance r. */
inline float RadialVelocity(float force, float mass, float r)
{
    return sqrtf(force * r / mass);
}

inline float PseudoIsothermal(float r, float rho0, float radius)
{
    return rho0 / (1.0f + (r / radius) * (r / radius));
}

inline float PlummerDensity(float r, float mass, float radius)
{
    return (3.0f * mass / (4.0f * PI * radius * radius * radius)) *
        (1.0f / sqrtf(pow((1.0f + (r * r) / (radius * radius)), 5.0f)));
}

inline float PlummerPotential(float r, float mass, float radius)
{
    return -mass / (sqrtf(r * r + radius * radius));
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

inline void IntegrateMotionEquation(float time, float3& position, float3& velocity, float3& acceleration, const float3& force, float inverseMass)
{
    // Euler-Cromer
    /*float3 a = acceleration + force * inverseMass;
    velocity += acceleration * time;
    position += velocity * time;*/
    acceleration += inverseMass * force;
    velocity += time * acceleration;
    position += time * velocity;
}