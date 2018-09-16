#pragma once

#include "lpVec3.h"

class GalaxyParticle {
public:

    bool			m_active;
    float			m_activationTime;
    float			m_timer;

    lpVec3			m_pos;
    lpVec3			m_linearVel;
    lpVec3			m_forceAccum;

    float			m_mass;
    float			m_invMass;

    float			m_cR;
    float			m_cG;
    float			m_cB;
    float			m_alpha;

    float			m_mag;
    float			m_size;

    unsigned int	m_texID;

    bool			m_doubleDrawing;

    int				m_userData;

    GalaxyParticle() = default;
    GalaxyParticle(float mass);

    void integrate(float dt);
    void setMass(float mass);

    lpVec3 caclGravityForce(const GalaxyParticle &other) const;
};

