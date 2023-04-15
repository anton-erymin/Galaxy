#pragma once

#include "GalaxyTypes.h"

class Universe;
class ISolver;
class IRendererPlugin;
class MainWindow;

class GalaxySimulator
{
public:
    GalaxySimulator();
    ~GalaxySimulator();

private:
    void CreateUniverse();
    void CreateSolver();
    void CreateRenderer();

private:
    SimulationContext sim_context_;
    RenderParameters render_params_;

    unique_ptr<Universe> universe_;
    unique_ptr<ISolver> solver_;
    unique_ptr<IRendererPlugin> renderer_;
    unique_ptr<MainWindow> main_window_;

    float cSecondsPerTimeUnit = 0;
    float cMillionYearsPerTimeUnit = 0;
    int32_t totalParticlesCount = 0;
    uint32_t frameCounter = 0;
    bool started = false;
    bool saveToFiles = false;

#if 0    
    GAL::ComputePipelinePtr particles_update_pipeline_;
    GAL::ComputePipelinePtr particles_clear_forces_pipeline_;
    GAL::ComputePipelinePtr particles_barnes_hut_pipeline_;
    GAL::ComputePipelinePtr particles_solve_pipeline_;
#endif // 0
};
