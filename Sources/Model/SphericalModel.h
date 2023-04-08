#pragma once

#include "MathUtils.h"

#include <vector>
#include <cassert>

class SphericalDistribution
{
public:
    SphericalDistribution(float mass = 1.0f, float radius = 1.0f) : mass(mass), radius(radius) { }
    virtual ~SphericalDistribution() = default;
    virtual float GetDensity(float r) const = 0;
    virtual float GetPotential(float r) const { assert(!"Not implemented"); return 0.0f; }

    float GetMass() const { return mass; }
    float GetRadius() const { return radius; }

protected:
    float mass;
    float radius;
};

class PlummerModel : public SphericalDistribution
{
public:
    PlummerModel(float mass = 1.0f, float radius = 1.0f) : SphericalDistribution(mass, radius) { }
    float GetDensity(float r) const override { return PlummerDensity(r, mass, radius); }
    float GetPotential(float r) const override { return PlummerPotential(r, mass, radius); }
};

// Dark matter halo spherical model
class SphericalModel
{
public:
	float	  rmin;
	float	  rmax;
	float	  h;

	float     radius;

    std::vector<float> rvec;
    std::vector<float> rho;
    std::vector<float> rightPartPoisson;
	std::vector<float> potential;
    std::vector<float> field;

	float	  vc;

	SphericalModel(float gridXMin, float gridXMax, float radius);

	// Расчет гравитационного потенциала
	void CalculatePotential();
	// Расчет напряженности
	void CalculateGravityField();

	float GetForce(float r) const;
	float GetCircularVelocity(float r) const;

	void PlotPotential() const;
};

