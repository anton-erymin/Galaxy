#include "Particle.h"

void Particle::SetMass(float mass)
{
    this->mass = mass;
    inverseMass = 1.0f / mass;
}
