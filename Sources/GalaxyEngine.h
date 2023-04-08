#pragma once

#include <EngineMinimal.h>
#include "Core/Galaxy.h"
#include "GalaxyUI.h"

class Universe;
class BruteforceSolver;
class BarnesHutCPUSolver;
class BarnesHutGPUSolver;
class Solver;

class GalaxyEngine final : public Engine
{
public:
    GalaxyEngine();
    ~GalaxyEngine();

    virtual void OnPostInitialize() override;

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

    const SimulationParameters& GetSimulationParamaters() const { return simulation_params_; }
    Timings& GetTimings() { return timings_; }

    static GalaxyEngine& GetInstance();

private:
    void CreateParticlesRenderPipelines();

    void Reset();

    void UpdateDeltaTime(float new_time);

    void Bind(GAL::PipelinePtr& pipeline);

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
    std::unique_ptr<BruteforceSolver> solverBruteforce;
    std::unique_ptr<BarnesHutCPUSolver> solverBarneshut;
    //std::unique_ptr<BarnesHutGPUSolver> solverBarneshutGPU;

    Solver* currentSolver = nullptr;

    std::thread solverThread;

    struct RenderParameters
    {
        bool renderTree = true;
        bool renderPoints = true;
        bool plotFunctions = false;
        float brightness = 1.0f;
        float particlesSizeScale = 1.0f;

    } renderParams;

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

    std::unique_ptr<UI::GalaxyUI> ui_;

    friend class UI::GalaxyUI;

    friend void CreateParticlesRenderPipelines(Renderer& r);
};
