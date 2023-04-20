#pragma once

#include "CPUSolverBase.h"

class BarnesHutCPUTree;

class BarnesHutCPUSolver : public CPUSolverBase
{
public:
    BarnesHutCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BarnesHutCPUSolver();

private:
    void TraverseTree(int32 node = -1, float radius = 0.0f);
    virtual void ComputeAcceleration() override;

private:
    unique_ptr<BarnesHutCPUTree> tree_;
};
