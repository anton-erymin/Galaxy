#include "GalaxyParticle.h"
#include "GalaxySystem.h"


GalaxyParticle::GalaxyParticle(float mass)
	: m_mass(mass),
	  m_invMass(1.0f / mass),
	  m_size(1.0f),
	  m_alpha(1.0f),
	  m_mag(1.0f),
	  m_cR(1.0f),
	  m_cG(1.0f),
	  m_cB(1.0f),
	  m_active(true),
	  m_doubleDrawing(false)
{
}


void GalaxyParticle::setMass(float mass)
{
	m_mass = mass;
	m_invMass = 1.0f / mass;
}


void GalaxyParticle::integrate(float dt)
{	
	m_linearVel.addScaled(m_forceAccum, m_invMass * dt);		
	m_pos.addScaled(m_linearVel, dt);
	m_forceAccum.clear();
}