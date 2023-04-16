#pragma once

#include "CPUSolverBase.h"

class BarnesHutCPUTree;

class BarnesHutCPUSolver : public CPUSolverBase
{
public:
    BarnesHutCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BarnesHutCPUSolver();

    void Solve(float time) override;

private:
    void TraverseTree(const BarnesHutCPUTree& node);

private:
    unique_ptr<BarnesHutCPUTree> tree_;
};
