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
    void CreateGalaxy(const Float3& position, const Float3& velocity);
    void CreateSolver(SimulationAlgorithm algorithm);
    void CreateRenderer();

    void OnEvent(Event& e);

private:
    SimulationContext sim_context_;
    RenderParameters render_params_;

    UniquePtr<Universe> universe_;
    UniquePtr<SolverBase> solver_;
    UniquePtr<GalaxyRenderer> renderer_;
    UniquePtr<MainWindow> main_window_;
};
