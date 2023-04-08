#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"

Universe::Universe(float size)
    : size(size)
{
}

Galaxy& Universe::CreateGalaxy()
{
    std::unique_ptr<Galaxy> galaxy = std::make_unique<Galaxy>();
    AddGalaxy(galaxy);
    return *galaxies.back();
}

Galaxy& Universe::CreateGalaxy(const float3& position, const GalaxyParameters& parameters = {})
{
    std::unique_ptr<Galaxy> galaxy = std::make_unique<Galaxy>(position, parameters);
    AddGalaxy(galaxy);
    return *galaxies.back();
}

void Universe::AddGalaxy(std::unique_ptr<Galaxy>& galaxy)
{
    position.reserve(position.size() + galaxy->GetParticlesCount());
    velocity.reserve(position.size() + galaxy->GetParticlesCount());
    acceleration.reserve(position.size() + galaxy->GetParticlesCount());
    force.reserve(position.size() + galaxy->GetParticlesCount());
    inverseMass.reserve(position.size() + galaxy->GetParticlesCount());

    particles.reserve(position.size() + galaxy->GetParticlesCount());

    for (const auto& particle : galaxy->GetParticles())
    {
        position.push_back(particle.position);
        velocity.push_back(particle.linearVelocity);
        acceleration.push_back(particle.acceleration);
        force.push_back(particle.force);
        inverseMass.push_back(particle.inverseMass);

        //assert(particle.image);
        //imageToParticles[particle.image].push_back(position.size() - 1);

        particles.push_back(&particle);
    }

    galaxies.push_back(std::move(galaxy));

    NLOG("Galaxy added");
}

void Universe::SetRadialVelocitiesFromForce()
{
    for (size_t i = 0; i < particles.size(); ++i)
    {
        float3 relativePos = float3(position[i]) - particles[i]->galaxy->GetPosition();
        float3 v = { relativePos.z, 0.0f, -relativePos.x };
        v.normalize();

        //float radialFromHalo = RadialVelocity(halo.GetForce(relativePos.norm()), particles[i].mass, relativePos.norm());
        float radial = RadialVelocity(force[i].length(), particles[i]->mass, relativePos.length());
        v *= radial * cRadialVelocityFactor;// + radialFromHalo;
                    //float d = 0.1 * v.norm();
                    //v += lpVec3(d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f));

        velocity[i] = v;//{0,0,0};

    }
}
