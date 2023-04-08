#pragma once

#include "Constants.h"
#include "Galaxy.h"

#include <GALFwd.h>
#include <Entity.h>

class Universe;
class ISolver;

struct SimulationParameters
{
	bool darkMatter = false;
	float softFactor = cSoftFactor;
};

struct Timings
{
	float buildTreeTimeMsecs = 0.0f;
	float solvingTimeMsecs = 0.0f;
};

struct RenderParameters
{
    bool renderTree = true;
    bool renderPoints = true;
    bool plotFunctions = false;
    float brightness = 1.0f;
    float particlesSizeScale = 1.0f;
};

class GalaxySimulator
{
public:
    GalaxySimulator();
    ~GalaxySimulator();

	const SimulationParameters& GetSimulationParameters() const { return simulation_params_; }

	Timings& GetTimings() { return timings_; }

private:
    void CreateParticlesRenderPipelines();

    void Reset();

    void UpdateDeltaTime(float new_time);

    void Bind(GAL::GraphicsPipelinePtr& pipeline);

    SimulationParameters simulation_params_;
    Timings timings_;

    float cSecondsPerTimeUnit = 0;
    float cMillionYearsPerTimeUnit = 0;

    float deltaTime = 0.0f;
    float deltaTimeYears = 0.0f;
    float simulationTime = 0.0f;
    float simulationTimeMillionYears = 0.0f;

    float lastFps = 0.0f;
    int32_t numSteps = 0;
    int32_t totalParticlesCount = 0;

    uint32_t frameCounter = 0;

    bool started = false;
    bool saveToFiles = false;

    std::unique_ptr<Universe> universe;
    //std::unique_ptr<BruteforceSolver> solverBruteforce;
    //std::unique_ptr<BarnesHutCPUSolver> solverBarneshut;
    //std::unique_ptr<BarnesHutGPUSolver> solverBarneshutGPU;

    //unique_ptr<ISolver> currentSolver;

    std::thread solverThread;

    RenderParameters renderParams;
    SimulationParameters simulationParams;
    GalaxyParameters model;

#if 0
    GAL::GraphicsPipelinePtr particles_render_pipeline_;
    GAL::GraphicsPipelinePtr tree_draw_pipeline_;
    GAL::ComputePipelinePtr particles_update_pipeline_;
    GAL::ComputePipelinePtr particles_clear_forces_pipeline_;
    GAL::ComputePipelinePtr particles_barnes_hut_pipeline_;
    GAL::ComputePipelinePtr particles_solve_pipeline_;
#endif // 0

    Entity particles_buffer_ = kInvalidEntity;
    Entity nodes_buffer_ = kInvalidEntity;
    GAL::BufferPtr nodes_counter;
    bool write_flag_ = false;

    bool is_simulated_ = false;

    Entity controller_ = kInvalidEntity;

    //friend void CreateParticlesRenderPipelines(Renderer& r);

};
