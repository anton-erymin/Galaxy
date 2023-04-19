#pragma once

enum class SimulationAlgorithm
{
    BRUTEFORCE_CPU,
    BRUTEFORCE_GPU,
    BARNESHUT_CPU,
    BARNESHUT_GPU,
    MAX_COUNT
};

struct SimulationContext
{
    SimulationAlgorithm algorithm = SimulationAlgorithm::BRUTEFORCE_CPU;

    bool is_simulated = true;

    float timestep;
    float half_timestep;
    float timestep_yrs;
    bool simulate_dark_matter = false;
    float gravity_softening_length;
    float barnes_hut_opening_angle;

    float cMillionYearsPerTimeUnit;
    float cSecondsPerTimeUnit;

    float simulation_fps = 0.0f;
    float simulation_time = 0.0f;
    float simulation_time_million_yrs = 0.0f;
    size_t timesteps_count = 0;
    size_t max_timesteps_count = 0;

    float build_tree_time_msecs = 0.0f;
    float compute_force_time_msecs = 0.0f;
    float total_step_time_msecs = 0.0f;

    atomic<size_t> nodes_count = 0;

    // Sync context
    atomic_bool positions_update_completed_flag = false;
    condition_variable solver_cv;
    mutex solver_mu;

    bool IsCPUAlgorithm() const { return algorithm == SimulationAlgorithm::BRUTEFORCE_CPU || algorithm == SimulationAlgorithm::BARNESHUT_CPU; }
    bool IsBarnesHut() const { return algorithm == SimulationAlgorithm::BARNESHUT_CPU || algorithm == SimulationAlgorithm::BARNESHUT_GPU; }
};

struct RenderParameters
{
    bool render_particles = true;
    bool render_as_points = true;
    bool render_tree = false;
    bool render_tracks = true;
    bool plot_potential = false;
    float brightness = 1.0f;
    float particle_size_scale = 7.0f;
    bool colors_inverted = true;
};
