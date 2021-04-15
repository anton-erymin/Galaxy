#pragma once

#include "Engine.h"
#include "Core/Galaxy.h"
#include "GalaxyUI.h"

class Universe;
class BruteforceSolver;
class BarnesHutCPUSolver;
class BarnesHutGPUSolver;
class Solver;

class GalaxyEngine : public Engine
{
public:
    GalaxyEngine(std::uint32_t width, std::uint32_t height, void* window_handle,
        const char* shaders_path);

    virtual ~GalaxyEngine() = default;

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

    const SimulationParameters& GetSimulationParamaters() const
    {
        return simulation_params_;
    }

    Timings& GetTimings()
    {
        return timings_;
    }

    static GalaxyEngine& GetInstance();

private:
    void Update(float time) override;
    void BuildUI() override;

    void PostRender() override;    

    void Reset();

    void UpdateDeltaTime(float new_time);

    void Bind(GL::Pipeline* pipeline);

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

    GL::GraphicsPipelinePtr particles_render_pipeline_;
    GL::GraphicsPipelinePtr tree_draw_pipeline_;
    GL::ComputePipelinePtr particles_update_pipeline_;
    GL::ComputePipelinePtr particles_clear_forces_pipeline_;
    GL::ComputePipelinePtr particles_barnes_hut_pipeline_;
    GL::ComputePipelinePtr particles_solve_pipeline_;
    Entity particles_buffer_ = kInvalidEntity;
    Entity nodes_buffer_ = kInvalidEntity;
    bool write_flag_ = false;

    bool is_simulated_ = false;

    Entity controller_ = kInvalidEntity;

    std::unique_ptr<UI::GalaxyUI> ui_;

    friend class UI::GalaxyUI;
};
