#include "BruteforceCPUSolver.h"
#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"

#include <Thread/Thread.h>
#include <Thread/ThreadPool.h>

#define USE_MULTITHREADING
#ifdef USE_MULTITHREADING
#define PARALLEL_FOR(count, kernel) ThreadPool::Dispatch(count, [&](size_t i) { kernel(i); });
#else
#define PARALLEL_FOR(count, kernel) for (size_t i = 0; i < count; i++) { kernel(i); }
#endif

BruteforceCPUSolver::BruteforceCPUSolver(Universe& universe)
    : ISolver(universe)
{
    active_flag_ = true;

    thread_.reset(new Thread("BruteforceCPUSolver Thread",
        [this]()
        {
            while (active_flag_)
            {
                Solve(0.0005f);
            }
        }));
}

BruteforceCPUSolver::~BruteforceCPUSolver()
{
    active_flag_ = false;
}

void BruteforceCPUSolver::Solve(float time)
{
    // Bruteforce

    size_t count = universe_.positions_.size();

    vector<mutex> mus(count);

    auto ComputeForceKernel = [&](size_t i)
    {
        assert(i < count);

        for (size_t j = i + 1; j < count; j++)
        {
            float3 l = float3(universe_.positions_[j]) - float3(universe_.positions_[i]);
            // TODO: Softended distance right?
            float dist = SoftenedDistance(l.length_sq(), cSoftFactor);
            float dist_cubic = dist * dist * dist;

            float3 force1 = GravityAcceleration(l, universe_.masses_[j], dist_cubic);
            float3 force2 = GravityAcceleration(-l, universe_.masses_[i], dist_cubic);

            {
                lock_guard<mutex> lock(mus[i]);
                universe_.forces_[i] += force1;
            }

            {
                lock_guard<mutex> lock(mus[j]);
                universe_.forces_[j] += force2;
            }
        }
    };

    auto IntegrationKernel = [&](size_t i)
    {
        assert(i < count);
        if (universe_.masses_[i] > 0.0f)
        {
            float3 pos = float3(universe_.positions_[i]);
            IntegrateMotionEquation(time, pos, universe_.velocities_[i],
                universe_.forces_[i], universe_.inverse_masses_[i]);
            universe_.positions_[i] = pos;
        }
        // Clear force accumulator
        universe_.forces_[i] = float3();
    };

    PARALLEL_FOR(count, ComputeForceKernel);
    PARALLEL_FOR(count, IntegrationKernel);
}
