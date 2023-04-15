#pragma once

#include "CPUSolverBase.h"

class BruteforceCPUSolver : public CPUSolverBase
{
public:
    BruteforceCPUSolver(Universe& universe, SimulationContext& context);
    ~BruteforceCPUSolver();

    void Solve(float time) override;
};
