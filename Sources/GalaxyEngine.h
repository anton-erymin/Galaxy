#pragma once

#include "Engine.h"
#include "Core/Galaxy.h"

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
        bool renderTree = false;
        bool renderPoints = true;
        bool plotFunctions = false;
        float brightness = 1.0f;
        float particlesSizeScale = 1.0f;

    } renderParams;

    SimulationParameters simulationParams;
    GalaxyParameters model;

    MeshData::Ptr particles_mesh_data_;
    Entity particles_vertex_buffer_ = kInvalidEntity;
    GL::PipelinePtr particles_pipeline_;
    bool write_flag_ = false;
};
