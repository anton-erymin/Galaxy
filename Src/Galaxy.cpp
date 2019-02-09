#include "Galaxy.h"

#include <string.h>
#include <gl\glut.h>
#include <Windows.h>

#include "Application.h"
#include "Constants.h"
#include "Image.h"
#include "Math.h"

extern int curDepth;

float **rhoData;
float maxp, minp;

float size;

int curLayer = 0;

Particle::Particle()
{
}

void Particle::SetMass(float mass)
{
    this->mass = mass;
    inverseMass = 1.0f / mass;
}

static void SortParticlesByImages(const std::vector<Particle>& particles, std::unordered_map<const Image*, std::vector<const Particle*>>& image_to_particles)
{
    image_to_particles.clear();
    for (const auto& particle : particles)
    {
        assert(particle.image);
        image_to_particles[particle.image].push_back(&particle);
    }
}

Galaxy::Galaxy() 
    : darkMatter(0.0f, 10.0f * GLX_HALO_RADIUS, GLX_HALO_RADIUS)
{
    bulgeRadius = GLX_BULGE_RADIUS;
    numBulgeStars = GLX_BULGE_NUM;
    bulgeMass = GLX_BULGE_MASS;
    diskRadius = GLX_DISK_RADIUS;
    numDiskStars = GLX_DISK_NUM;
    haloRadius = GLX_HALO_RADIUS;
    haloMass = GLX_HALO_MASS;
    diskThickness = GLX_DISK_THICKNESS;
    starMass = GLX_STAR_MASS;

    ccw = true;

    particles.reserve(numBulgeStars + numDiskStars);
    CreateBulge();
    CreateDisk();
    SortParticlesByImages(particles, image_to_particles);
}

Galaxy::Galaxy(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness,
    float bulgeMass, float haloMass, float starMass, bool ccw)
    : position(center),
    numBulgeStars(numBulgeStars),
    numDiskStars(numDiskStars),
    bulgeRadius(bulgeRadius),
    diskRadius(diskRadius),
    haloRadius(haloRadius),
    diskThickness(diskThickness),
    bulgeMass(bulgeMass),
    haloMass(haloMass),
    starMass(starMass),
    ccw(ccw),
    darkMatter(0.0f, 10.0f * haloRadius, haloRadius)
{
    particles.reserve(numBulgeStars + numDiskStars);
    CreateBulge();
    CreateDisk();
    SortParticlesByImages(particles, image_to_particles);
}

static Particle CreateStar()
{
    Particle particle;

    particle.size = RAND_RANGE(0.1f, 0.4f);
    particle.magnitude = RAND_RANGE(0.2f, 0.3f);

    int k = rand() % 3;
    float rnd = RAND;

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

    particle.image = &Application::GetInstance().GetImageLoader().GetImage("Star");
    
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

    particle.image = &Application::GetInstance().GetImageLoader().GetImage("Dust1");

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

void Galaxy::CreateBulge()
{
    int numDusts = numBulgeStars / 4;

    for (int i = 0; i < numBulgeStars; i++)
    {
        Particle particle;

        if (i < numDusts)
        {
            particle = CreateDust();
        }
        else
        {
            particle = CreateStar();
        }

        particle.SetMass(starMass);

        particle.position = position + SphericalToCartesian(RandomUniformSpherical(0, bulgeRadius));

        //speed = darkMatter.getCircularVelocity(r);
        //vel.setTo(pos.m_y, -pos.m_x, 0.0f);
        //vel.setTo(RAND_RANGE(-1.0f, 1.0f), RAND_RANGE(-1.0f, 1.0f), RAND_RANGE(-1.0f, 1.0f));
        //vel.normalize();
        //vel *= speed;

        //particle->linearVelocity = vel;

        particles.push_back(particle);
    }

    particles[0].position = position;
}

void Galaxy::CreateDisk()
{
    int numDusts = numDiskStars / 4;

    for (int i = 0; i < numDiskStars; i++)
    {
        Particle particle;

        if (i < numDusts)
        {
            particle = CreateDust();
        }
        else
        {
            particle = CreateStar();
        }

        particle.SetMass(starMass);

        particle.position = position + CylindricalToCartesian(RandomUniformCylindrical(0.99f * bulgeRadius, diskRadius, diskThickness));

        //speed = darkMatter.getCircularVelocity(r);
        //vel.setTo(pos.m_y, -pos.m_x, 0.0f);
        //vel.normalize();
        //vel *= speed;
        //d = 0.1 * speed;
        //vel += lpVec3(d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f), d * RAND_RANGE(-1.0f, 1.0f));

        //particle->linearVelocity = vel;

        particles.push_back(particle);
    }
}

void Galaxy::update(float dt)
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
{
    barnesHutTree = std::make_unique<BarnesHutTree>(lpVec3(-size * 0.5f), size);
}

Galaxy& Universe::CreateGalaxy()
{
    Galaxy galaxy;
    galaxies.push_back(std::move(galaxy));
    return galaxies.back();
}

Galaxy& Universe::CreateGalaxy(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness, float bulgeMass, float haloMass, float starMass, bool ccw)
{
    Galaxy galaxy{ center, numBulgeStars, numDiskStars, bulgeRadius, diskRadius, haloRadius, diskThickness, bulgeMass, haloMass, starMass, ccw };
    galaxies.push_back(std::move(galaxy));
    return galaxies.back();
}
