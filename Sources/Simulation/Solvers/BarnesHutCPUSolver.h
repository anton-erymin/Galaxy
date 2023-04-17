#pragma once

#include "CPUSolverBase.h"

class BarnesHutCPUTree;

class BarnesHutCPUSolver : public CPUSolverBase
{
public:
    BarnesHutCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BarnesHutCPUSolver();

private:
    void TraverseTree(const BarnesHutCPUTree& node);
    virtual void ComputeAcceleration() override;

private:
    unique_ptr<BarnesHutCPUTree> tree_;
};
