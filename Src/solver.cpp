#include "Solver.h"
#include "Galaxy.h"
#include "Math.h"
#include "Threading.h"
#include "BarnesHutTree.h"
#include "Constants.h"
#include "Utils.h"
#include "Application.h"

#include <cassert>

static void IntegrateMotionEquationSIMD(Particle& particle, float time)
{
    float *force = &particle.force.m_x;
    float *linVel = &particle.linearVelocity.m_x;
    float *pos = &particle.position.m_x;
    float imdt = particle.inverseMass * time;

    //__asm
    //{ 
    //	mov			eax, force
    //	movups		xmm0, [eax]		// xmm0 - force
    //	mov			ebx, linVel
    //	movups		xmm1, [ebx]		// xmm1 - velocity
    //	mov			ecx, pos
    //	movups		xmm2, [ecx]		// xmm2 - pos
    //	
    //	movss		xmm3, imdt
    //	movss		xmm4, dt
    //	shufps		xmm3, xmm3, 0	// xmm3 - imdt imdt imdt imdt
    //	shufps		xmm4, xmm4, 0	// xmm4 - dt dt dt dt

    //	mulps		xmm0, xmm3		// (dv) force *= imdt
    //	addps		xmm1, xmm0		// velocity += dv
    //	movups		xmm5, xmm1		// xmm5 - vel
    //	mulps		xmm1, xmm4		// (dp) velocity *= dt
    //	addps		xmm2, xmm1		// pos += dp

    //	xorps		xmm0, xmm0		// force = 0

    //	movups		[eax], xmm0
    //	movups		[ebx], xmm5
    //	movups		[ecx], xmm2
    //}
}

void BruteforceSolver::Solve(float time)
{
    // Bruteforce

    /*size_t ig = 0;
    for (auto &galaxy1 : universe.GetGalaxies()) 
    {
        size_t ip = 0;

        for (auto &particle1 : galaxy1.GetParticles()) 
        {
            for (size_t jp = ip + 1; jp < galaxy1.GetParticles().size(); ++jp) 
            {
                Particle& particle2 = galaxy1.GetParticles()[jp];
                lpVec3 force = GravityAcceleration(particle2.position - particle1.position,  );
                lpVec3 force = GravityAcceleration(particle2.position - particle1.position,  );

                particle1.acceleration += force;
                particle2.acceleration -= force;
            }

            for (size_t gj = ig + 1; gj < galaxies.size(); ++gj) {
                for (auto &p2 : galaxies[gj].particles()) {
                    auto force = p1.caclGravityForce(p2);

                    p1.m_forceAccum += force;
                    p2.m_forceAccum -= force;
                }
            }
            ++ip;
        }
        ++ig;
    }

    for (auto& galaxy : universe.GetGalaxies())
    {
        for (auto& particle : galaxy.GetParticles())
        {
            IntegrateMotionEquation(particle, time);
        }
    }*/
}

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

BarnesHutCPUSolver::BarnesHutCPUSolver(Universe& universe)
    : Solver(universe)
{
}

void BarnesHutCPUSolver::Solve(float time)
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
            auto velocity = universe.velocity[i];
            auto acceleration = universe.acceleration[i];
            auto force = universe.force[i];
            auto inverseMass = universe.inverseMass[i];
            ComputeForce(position, acceleration, force, *particle.galaxy, *barnesHutTree);
            IntegrateMotionEquation(time, position, velocity, acceleration, force, inverseMass);
            universe.position[i] = position;
            universe.velocity[i] = velocity;
        }

    }, static_cast<uint32_t>(universe.GetParticlesCount()), 
        static_cast<uint32_t>(universe.GetParticlesCount() / ThreadPool::GetThreadCount()));
}

void BarnesHutCPUSolver::SolveForces()
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

void BarnesHutCPUSolver::Inititalize(float time)
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

void BarnesHutCPUSolver::BuildTree()
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
