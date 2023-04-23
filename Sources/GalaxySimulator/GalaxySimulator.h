#pragma once

#include "GalaxyTypes.h"

#include <Events/EventHandler.h>

class Universe;
class SolverBase;
class GalaxyRenderer;
class MainWindow;

class GalaxySimulator : public IEventHandler
{
public:
    GalaxySimulator();
    ~GalaxySimulator();

private:
    void CreateUniverse();
    void CreateGalaxy(const float3& position, const float3& velocity);
    void CreateSolver(SimulationAlgorithm algorithm);
    void CreateRenderer();

    void OnEvent(Event& e);

private:
    SimulationContext sim_context_;
    RenderParameters render_params_;

    unique_ptr<Universe> universe_;
    unique_ptr<SolverBase> solver_;
    unique_ptr<GalaxyRenderer> renderer_;
    unique_ptr<MainWindow> main_window_;
};
