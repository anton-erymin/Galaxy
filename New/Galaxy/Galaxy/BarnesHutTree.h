#pragma once

#include "lpVec3.h"
#include "GalaxyParticle.h"


class BarnesHutTree
{
public:

	lpVec3				m_point;
	lpVec3				m_oppositePoint;
	float				m_length;

	GalaxyParticle	   *m_particle;

	float				m_totalMass;
	lpVec3				m_mc;

	bool				m_leaf;
	BarnesHutTree	  **m_subs;




	BarnesHutTree(const lpVec3 &point, float length);

	void insert(GalaxyParticle *p);
	void calcForce(GalaxyParticle *p);

private:
	bool inline contains(GalaxyParticle *p);

};

