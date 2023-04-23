#include "CPUSolverBase.h"
#include "GalaxySimulator/GalaxyTypes.h"
#include "Universe.h"

#include <Thread/Thread.h>

CPUSolverBase::CPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : SolverBase(universe, context, render_params)
    , force_mutexes_(universe.GetParticlesCount())
{
}

CPUSolverBase::~CPUSolverBase()
{
    Stop();
}

void CPUSolverBase::Start()
{
    active_flag_ = true;
    thread_.reset(new Thread("CPUSolver Thread", bind(&CPUSolverBase::SolverRun, this)));
}

void CPUSolverBase::Stop()
{
    if (active_flag_)
    {
        active_flag_ = false;
        context_.solver_cv.notify_one();
        thread_.reset();
    }
}

void CPUSolverBase::SolverRun()
{
    while (active_flag_)
    {
        if (!context_.is_simulated)
        {
            constexpr float SLEEP_INACTIVE_TIME_SECS = 0.1f;
            Thread::Sleep(SLEEP_INACTIVE_TIME_SECS);
            continue;
        }

        Update();

        if (context_.max_timesteps_count > 0 && context_.timesteps_count == context_.max_timesteps_count)
        {
            break;
        }
    }
}

void CPUSolverBase::IntegrateLeapFrogKickDrift()
{
    PARALLEL_FOR_METHOD_BIND(universe_.positions_.size(), CPUSolverBase::LeapFrogKickDriftIntegrationKernel);
}

void CPUSolverBase::IntegrateLeapFrogKick()
{
    PARALLEL_FOR_METHOD_BIND(universe_.positions_.size(), CPUSolverBase::LeapFrogKickIntegrationKernel);
}

void CPUSolverBase::LeapFrogKickDriftIntegrationKernel(THREAD_POOL_KERNEL_ARGS)
{
    if (universe_.masses_[global_id] > 0.0f)
    {
        float3 pos = float3(universe_.positions_[global_id]);

        // 1/2 kick
        universe_.velocities_[global_id] += universe_.forces_[global_id] * context_.half_timestep;
        // Drift
        pos += universe_.velocities_[global_id] * context_.timestep;
        universe_.positions_[global_id] = pos;
    }
    // Clear force accumulator
    universe_.forces_[global_id] = float3();
}

void CPUSolverBase::LeapFrogKickIntegrationKernel(THREAD_POOL_KERNEL_ARGS)
{
    if (universe_.masses_[global_id] > 0.0f)
    {
        // 1/2 kick
        universe_.velocities_[global_id] += universe_.forces_[global_id] * context_.half_timestep;
    }
}
