#pragma once

struct Particle;
class Galaxy;
struct GalaxyParameters;

class Universe
{
public:
    Universe(float size);

    Galaxy& CreateGalaxy();
    Galaxy& CreateGalaxy(const float3& position, const GalaxyParameters& parameters);

    void SetRadialVelocitiesFromForce();

    float GetSize() const { return size; }
    std::vector<std::unique_ptr<Galaxy>>& GetGalaxies() { return galaxies; }
    const std::vector<const Particle*>& GetParticles() const { return particles; }

    //const std::unordered_map<const Image*, std::vector<size_t>>& GetParticlesByImage() const { return imageToParticles; }

    size_t GetParticlesCount() const { return particles.size(); }

private:
    void AddGalaxy(std::unique_ptr<Galaxy>& galaxy);

    float size;

    std::vector<std::unique_ptr<Galaxy>> galaxies;

public:
    std::vector<float4> position;
    std::vector<float3> velocity;
    std::vector<float3> acceleration;
    std::vector<float3> force;
    std::vector<float> inverseMass;

    std::vector<const Particle*> particles;
private:

    //std::unordered_map<const Image*, std::vector<size_t>> imageToParticles;
};
