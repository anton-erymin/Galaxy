#include "BruteforceCPUSolver.h"
#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"
#include "GalaxySimulator/GalaxyTypes.h"

#include <Thread/ThreadPool.h>

BruteforceCPUSolver::BruteforceCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : CPUSolverBase(universe, context, render_params)
{
}

BruteforceCPUSolver::~BruteforceCPUSolver()
{
}

void BruteforceCPUSolver::ComputeAcceleration()
{
    // Bruteforce

    size_t count = universe_.positions_.size();

    auto ComputeForceKernel = [&](size_t global_id, size_t local_id, size_t block_id, size_t thread_id)
    {
        assert(global_id < count);

        for (size_t j = global_id + 1; j < count; j++)
        {
            float3 l = float3(universe_.positions_[j]) - float3(universe_.positions_[global_id]);
            // TODO: Softened distance right?
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

    PARALLEL_FOR(count, ComputeForceKernel);
}
