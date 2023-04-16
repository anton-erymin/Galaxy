#include "CPUSolverBase.h"
#include "GalaxySimulator/GalaxyTypes.h"
#include "Universe.h"
#include "MathUtils.h"

#include <Thread/Thread.h>
#include <Misc/FPSCounter.h>

CPUSolverBase::CPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : ISolver(universe, context, render_params)
    , force_mutexes_(universe.GetParticlesCount())
{
}

CPUSolverBase::~CPUSolverBase()
{
    active_flag_ = false;
    context_.solver_cv.notify_one();
    thread_.reset();
}

void CPUSolverBase::Start()
{
    active_flag_ = true;
    thread_.reset(new Thread("CPUSolver Thread", bind(&CPUSolverBase::SolverRun, this)));
}

void CPUSolverBase::SolverRun()
{
    FPSCounter fps_counter;

    while (active_flag_)
    {
        if (!context_.is_simulated)
        {
            constexpr float SLEEP_INACTIVE_TIME_SECS = 0.1f;
            Thread::Sleep(SLEEP_INACTIVE_TIME_SECS);
            continue;
        }

        float& time = context_.timestep;

        fps_counter.BeginFrame();

        Solve(time);

        fps_counter.EndFrame();

        context_.simulation_time += time;
        context_.simulation_time_million_yrs = context_.simulation_time * context_.cMillionYearsPerTimeUnit;
        ++context_.timesteps_count;
        context_.simulation_fps = fps_counter.GetFPS();
        context_.timestep_yrs = context_.timestep * context_.cMillionYearsPerTimeUnit * 1e6f;
    }
}

void CPUSolverBase::IntegrationKernel(THREAD_POOL_KERNEL_ARGS)
{
    assert(global_id < universe_.GetParticlesCount());

    if (universe_.masses_[global_id] > 0.0f)
    {
        float3 pos = float3(universe_.positions_[global_id]);

        IntegrateMotionEquation(context_.timestep, pos, universe_.velocities_[global_id],
            universe_.forces_[global_id], universe_.inverse_masses_[global_id]);
        universe_.positions_[global_id] = pos;
    }
    // Clear force accumulator
    universe_.forces_[global_id] = float3();
}
