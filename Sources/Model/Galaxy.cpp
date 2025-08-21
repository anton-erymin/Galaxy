#include "Galaxy.h"
#include "Constants.h"
#include "MathUtils.h"

#include <Math/Math.h>

static Particle CreateStar()
{
    Particle particle;

    particle.size = RandRange(0.1f, 0.4f);
    particle.magnitude = RandRange(0.2f, 0.3f);

    int k = rand() % 3;
    float rnd = RandNorm();

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

    particle.size = RandRange(4.0f, 7.5f);
    particle.magnitude = RandRange(0.015f, 0.02f);
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

    particle.size = RandRange(0.2f, 0.6f);
    particle.magnitude = RandRange(0.0f, 1.0f);

    particle.color = { 1.0f, 0.6f, 0.6f };

    particle.userData = rand() % 2;

    return particle;
}

Galaxy::Galaxy(const Float3& position, const GalaxyParameters& parameters)
    : position_(position)
    , parameters_(parameters)
{
    //halo_ = SphericalModel(0.0f, 2.0f * parameters.halo_radius, parameters.halo_radius);

    Create();
}

Galaxy::~Galaxy()
{
}

void Galaxy::Create()
{
    particles_.Reserve(parameters_.bulge_particles_count + parameters_.disk_particles_count);

    NASSERT(parameters_.disk_mass_ratio > 0.0f && parameters_.disk_mass_ratio <= 1.0f);

    const float bulge_particle_mass = 0.1f;// (1.0f - parameters_.disk_mass_ratio)* parameters_.total_mass / parameters_.bulge_particles_count;
    const float disk_particle_mass = 0.1f;// parameters_.disk_mass_ratio * parameters_.total_mass / parameters_.disk_particles_count;

    const float dust_ratio = 0.1f;
    uint32 dusts_count = static_cast<uint32>(parameters_.bulge_particles_count * dust_ratio);

    PlummerModel plummer;
    
    for (uint32 i = 0; i < parameters_.bulge_particles_count; ++i)
    {
        Particle particle = i < dusts_count ? CreateDust() : CreateStar();
        particle.SetMass(bulge_particle_mass);
        Float3 spherical = RandomUniformSpherical(0.0f, parameters_.bulge_radius);
        float r = SampleDistribution(0.0f, 1.0f, plummer.GetDensity(0.0f), [&plummer](float x) { return plummer.GetDensity(x); }) / 1.0f;
        spherical.x = r * parameters_.bulge_radius;
        Math::SphericalToCartesian(spherical, particle.position);
        particle.position += position_;
        particle.galaxy = this;
        particles_.PushBack(particle);
    }

    dusts_count = static_cast<uint32>(parameters_.disk_particles_count * dust_ratio);

    for (uint32 i = 0; i < parameters_.disk_particles_count; i++)
    {
        Particle particle = i < dusts_count ? CreateDust() : CreateStar();
        particle.SetMass(disk_particle_mass);
        Float3 cylindrical = RandomUniformCylindrical(parameters_.bulge_radius, parameters_.disk_radius, parameters_.disk_thickness);
        //float r = SampleDistribution(0.0f, 1.0f, plummer.GetDensity(0.0f), [&plummer](float x) { return plummer.GetDensity(x); }) / 1.0f;
        //cylindrical.x = r * parameters_.disk_radius;
        Float3 pos = CylindricalToCartesian(cylindrical);
        Swap(pos.y, pos.z);

        Float3 dir = Normalize(pos);
        dir.y = 0.0f;
        //Float3 ortho_dir = Math::Orthogonalize(dir, Math::Y);
        Float3 ortho_dir = Float3(dir.z, 0.0f, -dir.x);

        particle.position = position_ + pos;
        particle.velocity = RadialVelocity(parameters_.black_hole_mass, dir.Length()) * ortho_dir;
        particle.galaxy = this;
        particles_.EmplaceBack(particle);
    }

    if (!particles_.IsEmpty())
    {
        particles_[0].position = position_;
        particles_[0].velocity = Float3();
        particles_[0].SetMass(parameters_.black_hole_mass);
    }

    NLOG("Galaxy created");
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
                p->activationTime = RandRange(50.0f, 500.0f) * dt;
            }
        }
    }*/
}
