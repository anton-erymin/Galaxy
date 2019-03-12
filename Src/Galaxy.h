#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <numeric>

#include "float3.h"
#include "SphericalModel.h"
#include "Constants.h"

struct Image;
class Galaxy;

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

    Galaxy* galaxy = nullptr;

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

    const float3& GetPosition() const { return position; }
    std::vector<Particle>& GetParticles() { return particles; }
    const SphericalModel& GetHalo() const { return halo; }
    size_t GetParticlesCount() const { return particles.size(); }
    const GalaxyParameters& GetParameters() const { return parameters; }

private:
    void Create();

    float3 position;
    GalaxyParameters parameters;

    std::vector<Particle> particles;

    SphericalModel halo;
};

class Universe
{
public:
    Universe(float size);

    Galaxy& CreateGalaxy();
    Galaxy& CreateGalaxy(const float3& position, const GalaxyParameters& parameters);

    void SetRadialVelocitiesFromForce();

    float GetSize() const { return size; }
    std::vector<Galaxy>& GetGalaxies() { return galaxies; }
    const std::vector<const Particle*> GetParticles() const { return particles; }
    const std::unordered_map<const Image*, std::vector<size_t>> GetParticlesByImage() const { return imageToParticles; }

    size_t GetParticlesCount() const
    {
        return particles.size();
    }

private:
    void AddGalaxy(Galaxy& galaxy);

    float size;

    std::vector<Galaxy> galaxies;

public:
    std::vector<float3> position;
    std::vector<float3> velocity;
    std::vector<float3> acceleration;
    std::vector<float3> force;
    std::vector<float> inverseMass;

private:
    std::vector<const Particle*> particles;

    std::unordered_map<const Image*, std::vector<size_t>> imageToParticles;
};