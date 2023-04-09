#pragma once

#include "Constants.h"
#include "Particle.h"
#include "SphericalModel.h"

struct GalaxyParameters
{
    uint32_t disk_particles_count = GLX_DISK_NUM;
    uint32_t bulge_particles_count = GLX_BULGE_NUM;
    float total_mass = GLX_TOTAL_MASS;
    float disk_radius = GLX_DISK_RADIUS;
    float disk_thickness = GLX_DISK_THICKNESS;
    float disk_mass_ratio = GLX_DISK_MASS_RATIO;
    float bulge_radius = GLX_BULGE_RADIUS;
    float halo_radius = GLX_HALO_RADIUS;
    float black_hole_mass = GLX_BLACK_HOLE_MASS;
};

class Galaxy
{
public:
    Galaxy(const float3& position = {}, const GalaxyParameters& parameters = {});
    ~Galaxy();

    void Update(float dt);

    const float3& GetPosition() const { return position_; }
    vector<Particle>& GetParticles() { return particles_; }
    const SphericalModel& GetHalo() const { return halo_; }
    size_t GetParticlesCount() const { return particles_.size(); }
    const GalaxyParameters& GetParameters() const { return parameters_; }

private:
    void Create();

private:
    float3 position_;
    GalaxyParameters parameters_;
    vector<Particle> particles_;
    SphericalModel halo_;
};
