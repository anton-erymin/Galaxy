#include "Solver.h"
#include "Galaxy.h"
#include "Math.h"
#include "Threading.h"
#include "BarnesHutTree.h"

#include <cassert>

static inline lpVec3 CalculateGravityForce(const Particle& p1, const Particle& p2)
{
    lpVec3 force = p2.position;
    force -= p1.position;

    float r = force.norm();

    if (r < 50000.0f)
    {
        //r += 50000.0f;
    }

    force *= p1.mass * p2.mass / (r * r * r);

    return force;
}

static inline void IntegrateMotionEquation(Particle& particle, float time)
{
    // Euler-Cromer
    particle.linearVelocity.addScaled(particle.force, particle.inverseMass * time);
    particle.position.addScaled(particle.linearVelocity, time);
}

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

    size_t ig = 0;
    for (auto &galaxy1 : universe.GetGalaxies()) 
    {
        size_t ip = 0;

        for (auto &particle1 : galaxy1.GetParticles()) 
        {
            for (size_t jp = ip + 1; jp < galaxy1.GetParticles().size(); ++jp) 
            {
                Particle& particle2 = galaxy1.GetParticles()[jp];
                lpVec3 force = CalculateGravityForce(particle1, particle2);

                particle1.force += force;
                particle2.force -= force;
            }

            /*for (size_t gj = ig + 1; gj < galaxies.size(); ++gj) {
                for (auto &p2 : galaxies[gj].particles()) {
                    auto force = p1.caclGravityForce(p2);

                    p1.m_forceAccum += force;
                    p2.m_forceAccum -= force;
                }
            }*/
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
    }
}

void BarnesHutSolver::Solve(float time)
{
    barnesHutTree->Reset();
    {
        std::lock_guard<std::mutex> lock(mu);
        for (auto& galaxy : universe.GetGalaxies())
        {
            for (const auto& particle : galaxy.GetParticles())
            {
                barnesHutTree->Insert(particle);
            }
        }
    }

    // TODO: All galaxies
    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetGalaxies().front().GetParticles().size());

        Particle& particle = universe.GetGalaxies().front().GetParticles()[i];
        particle.force = barnesHutTree->CalculateForce(particle);
        IntegrateMotionEquation(particle, time);
    }, static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size()), 
        static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size() / ThreadPool::GetThreadCount()));
}

void BarnesHutSolver::Inititalize(float time)
{
    barnesHutTree = std::make_unique<BarnesHutTree>(lpVec3(-universe.GetSize() * 0.5f), universe.GetSize());

    {
        std::lock_guard<std::mutex> lock(mu);
        for (auto& galaxy : universe.GetGalaxies())
        {
            for (const auto& particle : galaxy.GetParticles())
            {
                barnesHutTree->Insert(particle);
            }
        }
    }

    float half = 0.5f * time;

    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetGalaxies().front().GetParticles().size());

        Particle& particle = universe.GetGalaxies().front().GetParticles()[i];
        particle.force = barnesHutTree->CalculateForce(particle);

        // Half step by velocity
        particle.linearVelocity += particle.force * particle.inverseMass * half;
        // Full step by position using half stepped velocity
        particle.position += particle.linearVelocity * time;

    }, static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size()), 
        static_cast<uint32_t>(universe.GetGalaxies().front().GetParticles().size() / ThreadPool::GetThreadCount()));
}
