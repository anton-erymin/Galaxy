#pragma once

#include "GPUSolverBase.h"

class BarnesHutGPUTree;

class BarnesHutGPUSolver : public GPUSolverBase
{
public:
    BarnesHutGPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BarnesHutGPUSolver();

private:
    virtual void ComputeAcceleration() override;

private:
    //unique_ptr<BarnesHutGPUTree> tree_;
};
