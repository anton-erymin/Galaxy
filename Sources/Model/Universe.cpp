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

void Universe::CreateGalaxy(const Float3& position, const GalaxyParameters& parameters)
{
    Galaxy galaxy(position, parameters);
    AddGalaxy(galaxy);
}

void Universe::AddGalaxy(Galaxy& galaxy)
{
    size_t count = galaxy.GetParticlesCount();

    positions_.Reserve(positions_.Size() + count);
    velocities_.Reserve(velocities_.Size() + count);
    //accelerations_.reserve(accelerations_.Size() + count);
    forces_.Reserve(forces_.Size() + count );
    inverse_masses_.Reserve(inverse_masses_.Size() + count);
    masses_.Reserve(masses_.Size() + count);

    all_particles_.Reserve(all_particles_.Size() + count);

    for (Particle& particle : galaxy.GetParticles())
    {
        positions_.PushBack(particle.position);
        velocities_.PushBack(particle.velocity);
        //accelerations_.PushBack(particle.acceleration);
        forces_.PushBack(particle.force);
        masses_.PushBack(particle.mass);
        inverse_masses_.PushBack(particle.inverse_mass);

        //NASSERT(particle.image);
        //imageToParticles[particle.image].PushBack(position.Size() - 1);

        all_particles_.PushBack(&particle);
    }

    //galaxies_.emplace_back(move(galaxy));

    NLOG("Galaxy added");
}

void Universe::SetRadialVelocitiesFromForce()
{
    for (size_t i = 0; i < all_particles_.Size(); ++i)
    {
        Float3 relativePos = Float3(positions_[i]) - all_particles_[i]->galaxy->GetPosition();
        Float3 v = { relativePos.z, 0.0f, -relativePos.x };
        v.Normalize();

        //float radialFromHalo = RadialVelocity(halo.GetForce(relativePos.norm()), particles[i].mass, relativePos.norm());
        float radial = RadialVelocity(forces_[i].Length(), all_particles_[i]->mass, relativePos.Length());
        v *= radial * cRadialVelocityFactor;// + radialFromHalo;
                    //float d = 0.1 * v.norm();
                    //v += lpVec3(d * RandRange(-1.0f, 1.0f), d * RandRange(-1.0f, 1.0f), d * RandRange(-1.0f, 1.0f));

        velocities_[i] = v;//{0,0,0};

    }
}

void Universe::SetRandomVelocities(float min, float max)
{
    for (size_t i = 0; i < velocities_.Size(); i++)
    {
        if (masses_[i] > 0.0f)
        {
            Float3 rand_dir(RandNormSigned(), 0.0f, RandNormSigned());
            rand_dir.Normalize();
            velocities_[i] = RandRange(min, max) * rand_dir;
        }
    }
}
