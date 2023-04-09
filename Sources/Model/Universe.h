#pragma once

struct Particle;
class Galaxy;
struct GalaxyParameters;

class Universe
{
public:
    Universe(float size);

    void CreateGalaxy();
    void CreateGalaxy(const float3& position, const GalaxyParameters& parameters);

    void SetRadialVelocitiesFromForce();

    float GetSize() const { return size_; }
    vector<Galaxy>& GetGalaxies() { return galaxies_; }
    const vector<const Particle*>& GetParticles() const { return all_particles_; }
    size_t GetParticlesCount() const { return all_particles_.size(); }

    //const unordered_map<const Image*, vector<size_t>>& GetParticlesByImage() const { return imageToParticles; }
public:
    vector<float4> positions_;
    vector<float3> velocities_;
    vector<float3> accelerations_;
    vector<float3> forces_;
    vector<float> inverse_masses_;

    vector<const Particle*> all_particles_;

private:
    void AddGalaxy(Galaxy& galaxy);

private:
    float size_;
    vector<Galaxy> galaxies_;

private:

    //unordered_map<const Image*, vector<size_t>> imageToParticles;
};
