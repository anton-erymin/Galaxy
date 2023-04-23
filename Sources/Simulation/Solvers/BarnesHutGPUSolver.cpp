#include "BruteforceCPUSolver.h"
#include "BarnesHutGPUSolver.h"

BarnesHutGPUSolver::BarnesHutGPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : GPUSolverBase(universe, context, render_params)
{
}

BarnesHutGPUSolver::~BarnesHutGPUSolver()
{
}

void BarnesHutGPUSolver::ComputeAcceleration()
{
}
