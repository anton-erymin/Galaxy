#pragma once

#include "GalaxyTypes.h"

#include <Events/EventHandler.h>

class Universe;
class ISolver;
class IRendererPlugin;
class MainWindow;

class GalaxySimulator : public IEventHandler
{
public:
    GalaxySimulator();
    ~GalaxySimulator();

private:
    void CreateUniverse();
    void CreateSolver(SimulationAlgorithm algorithm);
    void CreateRenderer();

    void OnEvent(Event& e);

private:
    SimulationContext sim_context_;
    RenderParameters render_params_;

    unique_ptr<Universe> universe_;
    unique_ptr<ISolver> solver_;
    unique_ptr<IRendererPlugin> renderer_;
    unique_ptr<MainWindow> main_window_;

#if 0    
    GAL::ComputePipelinePtr particles_update_pipeline_;
    GAL::ComputePipelinePtr particles_clear_forces_pipeline_;
    GAL::ComputePipelinePtr particles_barnes_hut_pipeline_;
    GAL::ComputePipelinePtr particles_solve_pipeline_;
#endif // 0
};
