#pragma once

#include "lpVec3.h"

#include <vector>

// Dark matter halo spherical model
class SphericalModel
{
public:
	float	  rmin;
	float	  rmax;
	float	  h;

	float     radius;

    std::vector<float> r;
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

	lpVec3 GetForce(lpVec3 pos) const;
	float  GetCircularVelocity(float r) const;

	void PlotPotential() const;
};

