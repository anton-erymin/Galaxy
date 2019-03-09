#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <numeric>

#include "float3.h"
#include "SphericalModel.h"
#include "Constants.h"

struct Image;

struct Particle
{
    bool active = true;
    float activationTime = 0;
    float timer = 0;

    float3 position = {};
    float3 linearVelocity = {};
    float3 acceleration = {};
    float3 force = {};

    float mass = 1.0f;
    float inverseMass = 1.0f;

    bool movable = true;

    float m_alpha;
    float3 color = { 1.0f, 1.0f, 1.0f };

    float magnitude = 1.0f;
    float size = 1.0f;

    const Image* image;

    bool doubleDrawing = false;

    int	userData = 0;

    void SetMass(float mass);
};

struct GalaxyParameters
{
    uint32_t diskParticlesCount = GLX_DISK_NUM;
    uint32_t bulgeParticlesCount = GLX_BULGE_NUM;
    float mass = GLX_TOTAL_MASS;
    float diskRadius = GLX_DISK_RADIUS;
    float diskThickness = GLX_DISK_THICKNESS;
    float diskMassRatio = GLX_DISK_MASS_RATIO;
    float bulgeRadius = GLX_BULGE_RADIUS;
    float haloRadius = GLX_HALO_RADIUS;
    float blackHoleMass = 1.0f;
};

class Galaxy
{
public:
    Galaxy(const float3& position = {}, const GalaxyParameters& parameters = {});

    void Update(float dt);

    std::vector<Particle>& GetParticles() { return particles; }
    const std::unordered_map<const Image*, std::vector<const Particle*>> GetParticlesByImage() const { return imageToParticles; }
    const SphericalModel& GetHalo() const { return halo; }
    size_t GetParticlesCount() const { return particles.size(); }

    void SetRadialVelocitiesFromForce();

private:
    void Create();

    float3 position;
    GalaxyParameters parameters;

    std::vector<Particle> particles;
    std::unordered_map<const Image*, std::vector<const Particle*>> imageToParticles;

    SphericalModel halo;
};

class Universe
{
public:
    Universe(float size);

    Galaxy& CreateGalaxy();
    Galaxy& CreateGalaxy(const float3& position, const GalaxyParameters& parameters);

    float GetSize() const { return size; }
    std::vector<Galaxy>& GetGalaxies() { return galaxies; }

    size_t GetParticlesCount() const
    {
        return std::accumulate(galaxies.begin(), galaxies.end(), 0ull, [](size_t sum, const Galaxy& galaxy) { return sum + galaxy.GetParticlesCount(); });
    }

private:
    float size;
    std::vector<Galaxy> galaxies;
};