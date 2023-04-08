#pragma once

#include "Constants.h"
#include "Particle.h"

class SphericalModel;

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
    float blackHoleMass = GLX_BLACK_HOLE_MASS;
};

class Galaxy
{
public:
    Galaxy(const float3& position = {}, const GalaxyParameters& parameters = {});
    ~Galaxy();

    void Update(float dt);

    const float3& GetPosition() const { return position; }
    std::vector<Particle>& GetParticles() { return particles; }
    const SphericalModel& GetHalo() const { return *halo; }
    size_t GetParticlesCount() const { return particles.size(); }
    const GalaxyParameters& GetParameters() const { return parameters; }

private:
    void Create();

private:
    float3 position;
    GalaxyParameters parameters;
    std::vector<Particle> particles;
    unique_ptr<SphericalModel> halo;
};
