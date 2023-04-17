#pragma once

#include "CPUSolverBase.h"

class BruteforceCPUSolver : public CPUSolverBase
{
public:
    BruteforceCPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BruteforceCPUSolver();

private:
    virtual void ComputeAcceleration() override;

};
