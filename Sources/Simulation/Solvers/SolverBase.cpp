#include "SolverBase.h"
#include "GalaxySimulator/GalaxyTypes.h"
#include "Universe.h"

#include <String/String.h>
#include <Engine.h>
#include <Interfaces/IDebugDrawSystem.h>

void SolverBase::Initialize()
{
    tracker_ = make_unique<BodyTracker>(universe_);

    // Initial acceleration
    ComputeAcceleration();
}

void SolverBase::Update()
{
    if (!context_.is_simulated)
    {
        return;
    }

    if (context_.max_timesteps_count > 0 && context_.timesteps_count == context_.max_timesteps_count)
    {
        return;
    }

    const float& time = context_.timestep;
    context_.half_timestep = 0.5f * time;

    fps_counter_.BeginFrame();

    Solve();

    fps_counter_.EndFrame();

    context_.simulation_time += time;
    context_.simulation_time_million_yrs = context_.simulation_time * context_.cMillionYearsPerTimeUnit;
    ++context_.timesteps_count;
    context_.simulation_fps = fps_counter_.GetFPS();
    context_.timestep_yrs = context_.timestep * context_.cMillionYearsPerTimeUnit * 1e6f;

    if (render_params_.render_tracks)
    {
        tracker_->Track();
    }
}

void SolverBase::Solve()
{
    float integration_time_part_1;
    float integration_time_part_2;

    BEGIN_TIME_MEASURE(total_timer, context_.total_step_time_msecs);

    // After computing force before integration phase wait for signal from renderer
    // that it has finished copying new positions into device buffer
    //unique_lock<mutex> lock(context_.solver_mu);
    //context_.solver_cv.wait(lock, [this]() { return context_.positions_update_completed_flag == false || !active_flag_; });

    // Leap-frog Kick-drift first part
    BEGIN_TIME_MEASURE(int_timer_1, integration_time_part_1);
    IntegrateLeapFrogKickDrift();
    END_TIME_MEASURE(int_timer_1);

    // After updating positions turn on flag signaling to renderer that it needs to update the device buffers
    context_.positions_update_completed_flag = true;

    BEGIN_TIME_MEASURE(acc_timer, context_.compute_acceleration_time_msecs);
    ComputeAcceleration();
    END_TIME_MEASURE(acc_timer);

    // Leap-frog Kick second part
    BEGIN_TIME_MEASURE(int_timer_2, integration_time_part_2);
    IntegrateLeapFrogKick();
    END_TIME_MEASURE(int_timer_2);

    context_.integration_time_msecs = integration_time_part_1 + integration_time_part_2;
}

void SolverBase::Dump(const char* prefix)
{
    size_t i = 1;
    NLOG("Step " << context_.timesteps_count << " [" << prefix << "]: "
        "pos: " << String::Float3ToStr(universe_.positions_[i]) <<
        ", vel: " << String::Float3ToStr(universe_.velocities_[i]) <<
        ", force: " << String::Float3ToStr(universe_.forces_[i]));
}

void BodyTracker::Track()
{
    for (size_t i = 0; i < universe_.GetParticlesCount(); i++)
    {
        const float3& new_pos = universe_.positions_[i];
        float dist_sq = (new_pos - prev_pos_[i]).length_sq();
        constexpr float TRACK_TRHESHOLD_DIST = 0.05f;
        constexpr float TRACK_TRHESHOLD_DIST_SQ = TRACK_TRHESHOLD_DIST * TRACK_TRHESHOLD_DIST;
        if (dist_sq > TRACK_TRHESHOLD_DIST_SQ)
        {
            engine->DebugDrawSystem()->DrawLine(prev_pos_[i], new_pos, Math::kRedColor, 0.5f, 0.0f, false);
            prev_pos_[i] = new_pos;
        }
    }
}

void BodyTracker::StorePositions()
{
    prev_pos_.resize(universe_.GetParticlesCount());
    for (size_t i = 0; i < prev_pos_.size(); i++)
    {
        prev_pos_[i] = universe_.positions_[i];
    }
}
