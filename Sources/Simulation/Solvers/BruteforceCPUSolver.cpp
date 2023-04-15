#include "BruteforceCPUSolver.h"
#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"
#include "GalaxySimulator/GalaxyTypes.h"

#include <Thread/ThreadPool.h>

BruteforceCPUSolver::BruteforceCPUSolver(Universe& universe, SimulationContext& context)
    : CPUSolverBase(universe, context)
{
}

BruteforceCPUSolver::~BruteforceCPUSolver()
{
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
    unique_lock<mutex> lock(context_.solver_mu);
    context_.solver_cv.wait(lock, [this]() { return context_.positions_update_completed_flag == false || !active_flag_; });

    // Now we can start update positions
    PARALLEL_FOR(count, IntegrationKernel);

    // After updating positions turn on flag signaling to renderer that it needs to update device buffer
    context_.positions_update_completed_flag = true;
}
