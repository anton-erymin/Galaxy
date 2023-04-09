#include "Particle.h"

void Particle::SetMass(float mass)
{
    this->mass = mass;
    inverse_mass = 1.0f / mass;
}
