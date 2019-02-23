#include "Solver.h"
#include "Galaxy.h"
#include "Math.h"
#include "Threading.h"

#include <cassert>

static lpVec3 CalculateGravityForce(const Particle& p1, const Particle& p2)
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

static void IntegrateMotionEquation(Particle& particle, float dt)
{
    particle.linearVelocity.addScaled(particle.force, particle.inverseMass * dt);
    particle.position.addScaled(particle.linearVelocity, dt);
    particle.force.clear();
}

static void IntegrateMotionEquationSIMD(Particle& particle, float dt)
{
    float *force = &particle.force.m_x;
    float *linVel = &particle.linearVelocity.m_x;
    float *pos = &particle.position.m_x;
    float imdt = particle.inverseMass * dt;

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

void BruteforceSolver::Solve(float dt, Universe& universe)
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
            IntegrateMotionEquation(particle, dt);
        }
    }
}

extern int curDepth;

void BarnesHutSolver::Solve(float dt, Universe& universe)
{
    universe.GetBarnesHutTree().Reset();

    for (auto& galaxy : universe.GetGalaxies())
    {
        for (const auto& particle : galaxy.GetParticles())
        {
            curDepth = 0;
            universe.GetBarnesHutTree().Insert(particle);
        }

        //curDepth = 0;
        //bht->Insert(particles[0]);
    }

    /*for (auto& galaxy : universe.GetGalaxies())
    {
        for (auto& particle : galaxy.GetParticles())
        {
            particle.force += universe.GetBarnesHutTree().CalculateForce(particle);
            IntegrateMotionEquationSIMD(particle, dt);
        }
    }*/

    ThreadPool().Dispatch([&](uint32_t i) 
    { 
        assert(i >= 0 && i < universe.GetGalaxies().front().GetParticles().size());

        Particle& particle = universe.GetGalaxies().front().GetParticles()[i];
        particle.force += universe.GetBarnesHutTree().CalculateForce(particle);
        IntegrateMotionEquationSIMD(particle, dt);
    }, universe.GetGalaxies().front().GetParticles().size(), universe.GetGalaxies().front().GetParticles().size() / ThreadPool::GetThreadCount());

    /*Galaxy* list = galaxies.data();
    for (auto& galaxy : galaxies)
    {
        galaxy.stepBarnesHutSIMD(dt, &*barnesHutTree, &list, galaxies.size());
    }*/
}


//void Galaxy::stepBarnesHutSIMD(float dt, BarnesHutTree *bht, Galaxy **galaxies, int numGalaxies)
//{
//    // —читаем силы взаимодействи¤ между звездами и ¤дром
//    for (auto& particle : particles)
//    {
//        bht->CalculateForce(particle);
//    }
//
//    // —читаем действие на звезды темного гало
//
//    for (int i = 0; i < numGalaxies; i++)
//    {
//        lpVec3 center = galaxies[i]->position;
//
//        for (auto& particle : particles)
//        {
//            lpVec3 pos = particle.position;
//            pos -= center;
//            //particles[j]->force.addScaled(galaxies[i]->darkMatter.getGravityVector(pos), particles[j]->mass);		
//        }
//    }
//
//    Particle* data = particles.data();
//    //stepParticles(dt, &data, particles.size());
//    update(dt);
//}