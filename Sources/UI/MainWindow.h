#pragma once

#include <Entity.h>

struct SimulationContext;
struct RenderParameters;

class MainWindow
{
public:
    MainWindow(SimulationContext& sim_context, RenderParameters& render_params);

private:
    void BuildUI();

private:
    SimulationContext& sim_context_;
    RenderParameters& render_params_;

    Entity window_;
};
