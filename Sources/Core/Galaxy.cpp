#include "Galaxy.h"

#include "Application.h"
#include "Constants.h"
#include "math_utils.h"

int curLayer = 0;

void Particle::SetMass(float mass)
{
    this->mass = mass;
    inverseMass = 1.0f / mass;
}

static Particle CreateStar()
{
    Particle particle;

    particle.size = RAND_RANGE(0.1f, 0.4f);
    particle.magnitude = RAND_RANGE(0.2f, 0.3f);

    int k = rand() % 3;
    float rnd = RAND_NORM;

    switch (k)
    {
    case 0:
        particle.color = { 1.0f, 1.0f - rnd * 0.2f, 1.0f - rnd * 0.2f };
        break;
    case 1:
        particle.color = { 1.0f, 1.0f, 1.0f - rnd * 0.2f };
        break;
    case 2:
        particle.color = { 1.0f - rnd * 0.2f, 1.0f - rnd * 0.2f, 1.0f };
        break;
    }

    return particle;
}

static Particle CreateDust()
{
    Particle particle;

    particle.size = RAND_RANGE(4.0f, 7.5f);
    particle.magnitude = RAND_RANGE(0.015f, 0.02f);
    //particle->size = 15;
    //particle.magnitude = 1;

    int k = rand() % 3;
    k = 1;

    if (k == 0)
    {
        particle.color = { 0.77f, 0.8f, 1.0f };
    }
    else
    {
        particle.color = { 1.0f, 0.95f, 0.8f };
    }

    return particle;
}

static Particle CreateH2()
{
    Particle particle;

    particle.size = RAND_RANGE(0.2f, 0.6f);
    particle.magnitude = RAND_RANGE(0.0f, 1.0f);

    particle.color = { 1.0f, 0.6f, 0.6f };

    particle.userData = rand() % 2;

    return particle;
}

Galaxy::Galaxy(const float3& position, const GalaxyParameters& parameters)
    : position(position)
    , parameters(parameters)
    , halo(0.0f, 2.0f * parameters.haloRadius, parameters.haloRadius)
{
    Create();
}

void Galaxy::Create()
{
    particles.reserve(parameters.bulgeParticlesCount + parameters.diskParticlesCount);

    assert(parameters.diskMassRatio > 0.0f && parameters.diskMassRatio < 1.0f);

    const float bulgeParticleMass = (1.0f - parameters.diskMassRatio) * parameters.mass / parameters.bulgeParticlesCount;
    const float diskParticleMass = parameters.diskMassRatio * parameters.mass / parameters.diskParticlesCount;

    const float dustRatio = 0.1f;

    uint32_t numDusts = static_cast<uint32_t>(parameters.bulgeParticlesCount * dustRatio);

    PlummerModel plummer;
    
    for (uint32_t i = 0; i < parameters.bulgeParticlesCount; ++i)
    {
        Particle particle = i < numDusts ? CreateDust() : CreateStar();
        particle.SetMass(bulgeParticleMass);
        float3 spherical = RandomUniformSpherical(0.0f, parameters.bulgeRadius);
        float r = SampleDistribution(0.0f, 1.0f, plummer.GetDensity(0.0f), [&plummer](float x) { return plummer.GetDensity(x); }) / 1.0f;
        spherical.x = r * parameters.bulgeRadius;
        particle.position = position + SphericalToCartesian(spherical);
        particle.galaxy = this;
        particles.push_back(particle);
    }

    numDusts = static_cast<uint32_t>(parameters.diskParticlesCount * dustRatio);

    for (uint32_t i = 0; i < parameters.diskParticlesCount; i++)
    {
        Particle particle = i < numDusts ? CreateDust() : CreateStar();
        particle.SetMass(diskParticleMass);
        float3 cylindrical = RandomUniformCylindrical(parameters.bulgeRadius, parameters.diskRadius, parameters.diskThickness);
        float r = SampleDistribution(0.0f, 1.0f, plummer.GetDensity(0.0f), [&plummer](float x) { return plummer.GetDensity(x); }) / 1.0f;
        cylindrical.x = r * parameters.diskRadius;
        float3 relativePos = CylindricalToCartesian(cylindrical);
        particle.position = position + relativePos;
        particle.galaxy = this;
        particles.push_back(particle);
    }

    particles[0].position = position;
    particles[0].movable = false;
    particles[0].SetMass(particles[0].mass * parameters.blackHoleMass);

    LOG("Galaxy created");
}

void Galaxy::Update(float dt)
{
    //position = particles[0]->position;


    /*float dmag = 0.01f;

    int off = m_numStars + m_numDusts;

    for (int i = 0; i < m_numH2; i++)
    {
        Particle *p = particles[i + off];

        if (!p->active)
        {
            p->timer += dt;
            if (p->timer > p->activationTime)
            {
                p->active = true;
                p->userData = 1;
            }
            continue;
        }

        if (p->userData == 1)
        {
            p->magnitude += dmag;
            if (p->magnitude > 1.0f)
            {
                p->magnitude = 1.0f;
                p->userData = 0;
            }
        }
        else
        {
            p->magnitude -= dmag;
            if (p->magnitude < 0.0f)
            {
                p->magnitude = 0.0f;
                p->active = false;
                p->timer = 0.0f;
                p->activationTime = RAND_RANGE(50.0f, 500.0f) * dt;
            }
        }
    }*/
}

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

        assert(particle.image);
        imageToParticles[particle.image].push_back(position.size() - 1);

        particles.push_back(&particle);
    }

    galaxies.push_back(std::move(galaxy));

    LOG("Galaxy added");
}

void Universe::SetRadialVelocitiesFromForce()
{
    for (size_t i = 0; i < particles.size(); ++i)
    {
        float3 relativePos = float3(position[i]) - particles[i]->galaxy->GetPosition();
        float3 v = {relativePos.y, -relativePos.x, 0.0f};
        v.normalize();
        
        //float radialFromHalo = RadialVelocity(halo.GetForce(relativePos.norm()), particles[i].mass, relativePos.norm());
        float radial = RadialVelocity(force[i].length(), particles[i]->mass, relativePos.length());
        v *= radial;// + radialFromHalo;
                    //float d = 0.1 * v.norm();
                    //v += lpVec3(d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f));

        velocity[i] = v;//{0,0,0};

    }
}