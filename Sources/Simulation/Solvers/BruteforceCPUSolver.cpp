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

    auto ComputeForceKernel = [&](size_t global_id, size_t local_id, size_t block_id, size_t thread_id)
    {
        assert(global_id < count);

        for (size_t j = global_id + 1; j < count; j++)
        {
            float3 l = float3(universe_.positions_[j]) - float3(universe_.positions_[global_id]);
            // TODO: Softended distance right?
            float dist = SoftenedDistance(l.length_sq(), context_.gravity_softening_length);
            float dist_cubic = dist * dist * dist;

            float3 force1 = GravityAcceleration(l, universe_.masses_[j], dist_cubic);
            float3 force2 = GravityAcceleration(-l, universe_.masses_[global_id], dist_cubic);

            {
                lock_guard<mutex> lock(force_mutexes_[global_id]);
                universe_.forces_[global_id] += force1;
            }

            {
                lock_guard<mutex> lock(force_mutexes_[j]);
                universe_.forces_[j] += force2;
            }
        }
    };

    auto IntegrationKernel = [&](size_t global_id, size_t local_id, size_t block_id, size_t thread_id)
    {
        assert(global_id < count);
        if (universe_.masses_[global_id] > 0.0f)
        {
            float3 pos = float3(universe_.positions_[global_id]);
            IntegrateMotionEquation(time, pos, universe_.velocities_[global_id],
                universe_.forces_[global_id], universe_.inverse_masses_[global_id]);
            universe_.positions_[global_id] = pos;
        }
        // Clear force accumulator
        universe_.forces_[global_id] = float3();
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
