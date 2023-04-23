#include "BruteforceCPUSolver.h"
#include "BruteforceGPUSolver.h"

BruteforceGPUSolver::BruteforceGPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
    : GPUSolverBase(universe, context, render_params)
{
}

BruteforceGPUSolver::~BruteforceGPUSolver()
{
}

void BruteforceGPUSolver::ComputeAcceleration()
{
}
