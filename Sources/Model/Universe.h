#pragma once

struct Particle;
class Galaxy;
struct GalaxyParameters;

class Universe
{
public:
    Universe();

    void CreateGalaxy();
    void CreateGalaxy(const Float3& position, const GalaxyParameters& parameters);

    void SetRadialVelocitiesFromForce();

    void SetRandomVelocities(float min, float max);

    Array<Galaxy>& GetGalaxies() { return galaxies_; }
    const Array<Particle*>& GetParticles() const { return all_particles_; }
    size_t GetParticlesCount() const { return positions_.Size(); }

    //const unordered_map<const Image*, Array<size_t>>& GetParticlesByImage() const { return imageToParticles; }

private:
    void AddGalaxy(Galaxy& galaxy);

private:
    Array<Galaxy> galaxies_;
    Array<Float4> positions_;
    Array<Float4> velocities_;
    //Array<Float3> accelerations_;
    Array<Float3> forces_;
    Array<float> masses_;
    Array<float> inverse_masses_;
    Array<Particle*> all_particles_;

    // Filled from CPU solver
    Array<Float4> node_positions_; // .w - node radius

    friend class SolverBase;
    friend class CPUSolverBase;
    friend class GPUSolverBase;
    friend class BruteforceCPUSolver;
    friend class BruteforceGPUSolver;
    friend class BarnesHutCPUSolver;
    friend class BarnesHutGPUSolver;
    friend class GalaxyRenderer;
    friend class GalaxySimulator;
    friend class BodyTracker;

    //unordered_map<const Image*, Array<size_t>> imageToParticles;
};
