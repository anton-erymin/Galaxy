#include "GPUSolver.h"
#include "Galaxy.h"
#include "Math.h"
#include "Threading.h"
#include "BarnesHutTree.h"
#include "Constants.h"
#include "Utils.h"
#include "Application.h"

#include <cassert>

static inline void ComputeForce(const float3& position, float3& acceleration, float3& force, const Galaxy& galaxy, const BarnesHutTree& tree)
{
    acceleration.clear();
    force.clear();

    acceleration = tree.ComputeAccelerationFlat(position, cSoftFactor);

    if (Application::GetInstance().GetSimulationParamaters().darkMatter)
    {    
        float darkMatterForce = galaxy.GetHalo().GetForce(position.norm());
        float3 forceDir = position;
        forceDir.normalize();
        force += forceDir * -darkMatterForce;
    }
}

BarnesHutGPUSolver::BarnesHutGPUSolver(Universe& universe)
    : Solver(universe)
{
}

void BarnesHutGPUSolver::Solve(float time)
{
    BuildTree();

    Timer<std::milli> timer(&Application::GetInstance().GetTimings().solvingTimeMsecs);

    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetParticlesCount());

        const Particle& particle = *universe.GetParticles()[i];

        if (particle.movable)
        {
            auto position = universe.position[i];
            auto acceleration = universe.acceleration[i];
            auto force = universe.force[i];

            ComputeForce(position, acceleration, force, *particle.galaxy, *barnesHutTree);

            universe.force[i] = force;
            universe.acceleration[i] = acceleration;

        }

    }, static_cast<uint32_t>(universe.GetParticlesCount()), 
        static_cast<uint32_t>(universe.GetParticlesCount() / ThreadPool::GetThreadCount()));

    cl.EnqueueWriteBuffer(acceleration, 0, acceleration->GetSize(), universe.acceleration.data(), false);
    cl.EnqueueWriteBuffer(force, 0, force->GetSize(), universe.force.data(), false);
    

    kernelIntegrate->SetArg(time, 0);
    cl.EnqueueKernel(kernelIntegrate, 1, universe.GetParticlesCount(), 0, 0, 256, 0, 0);
    cl.EnqueueBarrier();

    cl.EnqueueReadBuffer(position, 0, position->GetSize(), universe.position.data());
}

void BarnesHutGPUSolver::SolveForces()
{
    BuildTree();
    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetParticlesCount());

        const Particle& particle = *universe.GetParticles()[i];

        if (particle.movable)
        {
            ComputeForce(universe.position[i], universe.acceleration[i], universe.force[i], *particle.galaxy, *barnesHutTree);
            universe.force[i] += universe.acceleration[i] * particle.mass;
        }

    }, static_cast<uint32_t>(universe.GetParticlesCount()), 
        static_cast<uint32_t>(universe.GetParticlesCount() / ThreadPool::GetThreadCount()));
}

void BarnesHutGPUSolver::Inititalize(float time)
{
    barnesHutTree = std::make_unique<BarnesHutTree>(float3(-universe.GetSize() * 0.5f), universe.GetSize());

    BuildTree();

    float half = 0.5f * time;

    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetParticlesCount());

        const Particle& particle = *universe.GetParticles()[i];

        if (particle.movable)
        {
            ComputeForce(universe.position[i], universe.acceleration[i], universe.force[i], *particle.galaxy, *barnesHutTree);

            universe.acceleration[i].addScaled(particle.force, particle.inverseMass);
            // Half step by velocity
            universe.velocity[i] += universe.acceleration[i] * half;
            // Full step by position using half stepped velocity
            universe.position[i] += universe.velocity[i] * time;
        }

    }, static_cast<uint32_t>(universe.GetParticlesCount()), 
        static_cast<uint32_t>(universe.GetParticlesCount() / ThreadPool::GetThreadCount()));
}

void BarnesHutGPUSolver::Prepare()
{
    // Load programs
    programIntegrate = cl.CreateProgram(ReadFile("Kernels/Integrate.cl"));
    kernelIntegrate = programIntegrate->GetKernel("Integrate");

    size_t count = universe.GetParticlesCount();

    // Buffers
    position = cl.CreateBuffer(count * sizeof(float3));
    velocity = cl.CreateBuffer(count * sizeof(float3));
    acceleration = cl.CreateBuffer(count * sizeof(float3));
    force = cl.CreateBuffer(count * sizeof(float3));
    inverseMass = cl.CreateBuffer(count * sizeof(float3));

    // Kernel args
    kernelIntegrate->SetArg(&*position, 1);
    kernelIntegrate->SetArg(&*velocity, 2);
    kernelIntegrate->SetArg(&*acceleration, 3);
    kernelIntegrate->SetArg(&*force, 4);
    kernelIntegrate->SetArg(&*inverseMass, 5);

    cl.EnqueueWriteBuffer(position, 0, position->GetSize(), universe.position.data());
    cl.EnqueueWriteBuffer(velocity, 0, velocity->GetSize(), universe.velocity.data());
    cl.EnqueueWriteBuffer(inverseMass, 0, inverseMass->GetSize(), universe.inverseMass.data());
}

void BarnesHutGPUSolver::BuildTree()
{
    std::lock_guard<std::mutex> lock(mu);
    {
        Timer<std::milli> timer(&Application::GetInstance().GetTimings().buildTreeTimeMsecs);
        barnesHutTree->Reset();

        for (size_t i = 0; i < universe.GetParticlesCount(); ++i)
        {
            barnesHutTree->Insert(universe.position[i], universe.particles[i]->mass);
        }
    }
}
