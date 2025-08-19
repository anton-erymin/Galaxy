#pragma once

#include "Math/Math.h"

float integrate_rect(float a, float b, int n, float(*f)(float));
float integrate_trap(float a, float b, int n, float(*f)(float));

void Poisson1(uint32_t numIter, float min, float max, int n, float *data, const Array<float>& rightPart);

bool poisson1d(int numIter, float min, float max, int n, float   *data, float(*f)(float));
bool poisson2d(int numIter, float min, float max, int n, float  **data, float(*f)(float, float));
bool poisson3d(int numIter, float min, float max, int n, float ***data, float(*f)(float, float, float));

float RandomStandardDistribution();

Float3 SphericalToCartesian(float r, float phi, float theta);

Float3 SphericalToCartesian(const Float3& spherical);

Float3 CylindricalToCartesian(float r, float phi, float z);

Float3 CylindricalToCartesian(const Float3& cylindrical);

Float3 RandomUniformSpherical(float rmin, float rmax);

Float3 RandomUniformCylindrical(float rmin, float rmax, float height);

float SoftenedDistance(float dist_squared, float soft_factor);
Float3 GravityAcceleration(const Float3& l, float mass, float soft, float length_sq);
Float3 GravityAcceleration(const Float3& l, float mass, float softened_dist_cubic);

/** Radial velocity about body with certain mass at distance r. */
float RadialVelocity(float mass, float r);

/** Radial velocity of body with mass in force field at distance r. */
float RadialVelocity(float force, float mass, float r);

float PseudoIsothermal(float r, float rho0, float radius);

float PlummerDensity(float r, float mass, float radius);

float PlummerPotential(float r, float mass, float radius);

void IntegrateMotionEquation(float time, Float3& position, Float3& velocity, 
    const Float3& force, float inverse_mass);

template <typename Distribution>
inline float SampleDistribution(float xmin, float xmax, float maxDistributionValue, Distribution distribution)
{
    float x = 0.0f, y = 0.0f;
    while (true)
    {
        x = RandRange(xmin, xmax);
        y = RandRange(0.0f, maxDistributionValue);
        if (y < distribution(x))
        {
            break;
        }
    }
    return x;
}
