#include "GPUSolver.h"
#include "Galaxy.h"
#include "Math.h"
#include "Threading.h"
#include "BarnesHutTree.h"
#include "Constants.h"
#include "Utils.h"
#include "Application.h"

#include <cassert>

static inline void IntegrateMotionEquation(Particle& particle, float time)
{
    // Euler-Cromer
    particle.acceleration.addScaled(particle.force, particle.inverseMass);
    particle.linearVelocity.addScaled(particle.acceleration, time);
    particle.position.addScaled(particle.linearVelocity, time);
}

static inline void ComputeForce(Particle& particle, const Galaxy& galaxy, const BarnesHutTree& tree)
{
    particle.acceleration.clear();
    //particle.acceleration = tree.ComputeAcceleration(particle, cSoftFactor);

    particle.force.clear();

    if (Application::GetInstance().GetSimulationParamaters().darkMatter)
    {    
        float darkMatterForce = galaxy.GetHalo().GetForce(particle.position.norm());
        float3 forceDir = particle.position;
        forceDir.normalize();
        particle.force += forceDir * -darkMatterForce;
    }
}

BarnesHutGPUSolver::BarnesHutGPUSolver(Universe& universe)
    : Solver(universe)
{
    // Load programs
    programIntegrate = cl.CreateProgram(ReadFile("Kernels/Integrate.cl"));
    kernelIntegrate = programIntegrate->GetKernel("Integrate");

    // Buffers
    bufParticles = cl.CreateBuffer(universe.GetParticlesCount() * sizeof(Particle));

    // Kernel args
    //kernelIntegrate->SetArg(&*bufParticles, 0);
}

void BarnesHutGPUSolver::Solve(float time)
{
    BuildTree();

    Timer<std::milli> timer(&Application::GetInstance().GetTimings().solvingTimeMsecs);
    // TODO: All galaxies
    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetGalaxies().front().GetParticles().size());

        Particle& particle = universe.GetGalaxies().front().GetParticles()[i];

        if (particle.movable)
        {
            ComputeForce(particle, universe.GetGalaxies().front(), *barnesHutTree);
        }

    }, static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size()), 
        static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size() / ThreadPool::GetThreadCount()));

    cl.EnqueueWriteBuffer(bufParticles, 0, bufParticles->GetSize(), universe.GetGalaxies().front().GetParticles().data());
    kernelIntegrate->SetArg(cl_float(time), 1);
    cl.EnqueueKernel(kernelIntegrate, 1, universe.GetGalaxies().front().GetParticles().size(), 0, 0, 256, 0, 0);
    cl.WaitIdle();
    cl.EnqueueReadBuffer(bufParticles, 0, bufParticles->GetSize(), universe.GetGalaxies().front().GetParticles().data());
}

void BarnesHutGPUSolver::SolveForces()
{
    BuildTree();
    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetGalaxies().front().GetParticles().size());

        Particle& particle = universe.GetGalaxies().front().GetParticles()[i];

        if (particle.movable)
        {
            ComputeForce(particle, universe.GetGalaxies().front(), *barnesHutTree);
            particle.force += particle.acceleration * particle.mass;
            particle.acceleration.clear();
        }

    }, static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size()), 
        static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size() / ThreadPool::GetThreadCount()));
}

void BarnesHutGPUSolver::Inititalize(float time)
{
    barnesHutTree = std::make_unique<BarnesHutTree>(float3(-universe.GetSize() * 0.5f), universe.GetSize());

    BuildTree();

    float half = 0.5f * time;

    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetGalaxies().front().GetParticles().size());

        Particle& particle = universe.GetGalaxies().front().GetParticles()[i];
        if (particle.movable)
        {
            ComputeForce(particle, universe.GetGalaxies().front(), *barnesHutTree);
            particle.acceleration.addScaled(particle.force, particle.inverseMass);
            // Half step by velocity
            particle.linearVelocity += particle.acceleration * half;
            // Full step by position using half stepped velocity
            particle.position += particle.linearVelocity * time;
        }

    }, static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size()), 
        static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size() / ThreadPool::GetThreadCount()));
}

void BarnesHutGPUSolver::BuildTree()
{
    std::lock_guard<std::mutex> lock(mu);
    {
        Timer<std::milli> timer(&Application::GetInstance().GetTimings().buildTreeTimeMsecs);
        barnesHutTree->Reset();
        for (auto& galaxy : universe.GetGalaxies())
        {
            for (const auto& particle : galaxy.GetParticles())
            {
                //barnesHutTree->Insert(particle);
            }
        }
    }
}
