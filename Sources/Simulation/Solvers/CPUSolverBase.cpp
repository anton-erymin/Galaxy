#include "CPUSolverBase.h"
#include "GalaxySimulator/GalaxyTypes.h"
#include "Universe.h"
#include "MathUtils.h"

#include <Thread/Thread.h>
#include <Misc/FPSCounter.h>
#include <Engine.h>
#include "Interfaces/IDebugDrawSystem.h"

class ParticleTracker
{
public:
    ParticleTracker(Universe& universe)
        : universe_(universe)
    {
        StorePositions();
    }

    void AddTracks(initializer_list<size_t>& list)
    {
        track_indices_.insert(track_indices_.end(), list.begin(), list.end());
    }

    void Track()
    {
        for (size_t i = 0; i < universe_.GetParticlesCount(); i++)
        {
            const float3& new_pos = universe_.positions_[i];
            engine->DebugDrawSystem()->DrawLine(prev_pos_[i], new_pos, Math::kRedColor, 0.5f, 0.0f, false);
            prev_pos_[i] = new_pos;
        }
    }

    void StorePositions()
    {
        prev_pos_.resize(universe_.GetParticlesCount());
        for (size_t i = 0; i < prev_pos_.size(); i++)
        {
            prev_pos_[i] = universe_.positions_[i];
        }
    }

private:
    const Universe& universe_;
    vector<size_t> track_indices_;
    vector<float3> prev_pos_;
};

CPUSolverBase::CPUSolverBase(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : ISolver(universe, context, render_params)
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
    FPSCounter fps_counter;

    ParticleTracker tracker(universe_);

    ComputeAcceleration();

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

        //tracker.Track();

        if (context_.max_timesteps_count > 0 && context_.timesteps_count == context_.max_timesteps_count)
        {
            break;
        }
    }
}

void CPUSolverBase::Solve(float time)
{
    BEGIN_TIME_MEASURE(total_timer, context_.total_step_time_msecs);

    size_t count = universe_.positions_.size();

    context_.half_timestep = 0.5f * context_.timestep;

    // After computing force before integration phase wait for signal from renderer
    // that it has finished copying new positions into device buffer
    unique_lock<mutex> lock(context_.solver_mu);
    context_.solver_cv.wait(lock, [this]() { return context_.positions_update_completed_flag == false || !active_flag_; });

    // Leap-frog Kick-drift first part
    PARALLEL_FOR_METHOD_BIND(count, CPUSolverBase::LeapFrogKickDriftIntegrationKernel);

    // After updating positions turn on flag signaling to renderer that it needs to update the device buffers
    context_.positions_update_completed_flag = true;

    ComputeAcceleration();

    // Leap-frog Kick second part
    PARALLEL_FOR_METHOD_BIND(count, CPUSolverBase::LeapFrogKickIntegrationKernel);

    END_TIME_MEASURE(total_timer);
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

#include "String/String.h"
void CPUSolverBase::Dump(const char* prefix)
{
    size_t i = 1;
    NLOG("Step " << context_.timesteps_count << " [" << prefix << "]: "
        "pos: " << String::Float3ToStr(universe_.positions_[i]) << 
        ", vel: " << String::Float3ToStr(universe_.velocities_[i]) <<
        ", force: " << String::Float3ToStr(universe_.forces_[i]));
}
