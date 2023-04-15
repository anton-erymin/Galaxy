#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"

Universe::Universe()
{
}

void Universe::CreateGalaxy()
{
    Galaxy galaxy;
    AddGalaxy(galaxy);
}

void Universe::CreateGalaxy(const float3& position, const GalaxyParameters& parameters)
{
    Galaxy galaxy(position, parameters);
    AddGalaxy(galaxy);
}

void Universe::AddGalaxy(Galaxy& galaxy)
{
    size_t count = galaxy.GetParticlesCount();

    positions_.reserve(positions_.size() + count);
    velocities_.reserve(velocities_.size() + count);
    //accelerations_.reserve(accelerations_.size() + count);
    forces_.reserve(forces_.size() + count );
    inverse_masses_.reserve(inverse_masses_.size() + count);
    masses_.reserve(masses_.size() + count);

    all_particles_.reserve(all_particles_.size() + count);

    for (Particle& particle : galaxy.GetParticles())
    {
        positions_.push_back(particle.position);
        velocities_.push_back(particle.velocity);
        //accelerations_.push_back(particle.acceleration);
        forces_.push_back(particle.force);
        masses_.push_back(particle.mass);
        inverse_masses_.push_back(particle.inverse_mass);

        //assert(particle.image);
        //imageToParticles[particle.image].push_back(position.size() - 1);

        all_particles_.push_back(&particle);
    }

    //galaxies_.emplace_back(move(galaxy));

    NLOG("Galaxy added");
}

void Universe::SetRadialVelocitiesFromForce()
{
    for (size_t i = 0; i < all_particles_.size(); ++i)
    {
        float3 relativePos = float3(positions_[i]) - all_particles_[i]->galaxy->GetPosition();
        float3 v = { relativePos.z, 0.0f, -relativePos.x };
        v.normalize();

        //float radialFromHalo = RadialVelocity(halo.GetForce(relativePos.norm()), particles[i].mass, relativePos.norm());
        float radial = RadialVelocity(forces_[i].length(), all_particles_[i]->mass, relativePos.length());
        v *= radial * cRadialVelocityFactor;// + radialFromHalo;
                    //float d = 0.1 * v.norm();
                    //v += lpVec3(d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f));

        velocities_[i] = v;//{0,0,0};

    }
}
