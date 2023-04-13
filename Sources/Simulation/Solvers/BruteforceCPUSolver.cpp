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
    , force_mutexes_(universe.GetParticlesCount())
{
}

BruteforceCPUSolver::~BruteforceCPUSolver()
{
    active_flag_ = false;
    solver_cv_.notify_one();
    thread_.reset();
}

void BruteforceCPUSolver::Start()
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

void BruteforceCPUSolver::Solve(float time)
{
    // Bruteforce

    size_t count = universe_.positions_.size();

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
                lock_guard<mutex> lock(force_mutexes_[i]);
                universe_.forces_[i] += force1;
            }

            {
                lock_guard<mutex> lock(force_mutexes_[j]);
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

    // After computing force before integration phase wait for signal from renderer
    // that it has finished copying new positions into device buffer
    unique_lock<mutex> lock(solver_mu_);
    solver_cv_.wait(lock, [this]() { return *positions_update_completed_flag_ == false || !active_flag_; });

    // Now we can start update positions
    PARALLEL_FOR(count, IntegrationKernel);

    // After updating positions turn on flag signaling to renderer that it needs to update device buffer
    if (positions_update_completed_flag_)
    {
        *positions_update_completed_flag_ = true;
    }
}
