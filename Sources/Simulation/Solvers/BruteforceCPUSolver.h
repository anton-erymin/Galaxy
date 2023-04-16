#pragma once

#include "CPUSolverBase.h"

class BruteforceCPUSolver : public CPUSolverBase
{
public:
    BruteforceCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BruteforceCPUSolver();

    void Solve(float time) override;
};
