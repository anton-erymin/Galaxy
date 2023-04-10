#pragma once

#include "Math/Math.h"

float integrate_rect(float a, float b, int n, float(*f)(float));
float integrate_trap(float a, float b, int n, float(*f)(float));

void Poisson1(uint32_t numIter, float min, float max, int n, float *data, const vector<float>& rightPart);

bool poisson1d(int numIter, float min, float max, int n, float   *data, float(*f)(float));
bool poisson2d(int numIter, float min, float max, int n, float  **data, float(*f)(float, float));
bool poisson3d(int numIter, float min, float max, int n, float ***data, float(*f)(float, float, float));

float RandomStandardDistribution();

float3 SphericalToCartesian(float r, float phi, float theta);

float3 SphericalToCartesian(const float3& spherical);

float3 CylindricalToCartesian(float r, float phi, float z);

float3 CylindricalToCartesian(const float3& cylindrical);

float3 RandomUniformSpherical(float rmin, float rmax);

float3 RandomUniformCylindrical(float rmin, float rmax, float height);

float SoftenedDistance(float dist_squared, float soft_factor);
float3 GravityAcceleration(const float3& l, float mass, float soft, float length_sq);
float3 GravityAcceleration(const float3& l, float mass, float softened_dist_cubic);

/** Radial velocity about body with certain mass at distance r. */
float RadialVelocity(float mass, float r);

/** Radial velocity of body with mass in force field at distance r. */
float RadialVelocity(float force, float mass, float r);

float PseudoIsothermal(float r, float rho0, float radius);

float PlummerDensity(float r, float mass, float radius);

float PlummerPotential(float r, float mass, float radius);

void IntegrateMotionEquation(float time, float3& position, float3& velocity, 
    const float3& force, float inverse_mass);

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
