#pragma once

#include "GPUSolverBase.h"

class BruteforceGPUSolver : public GPUSolverBase
{
public:
    BruteforceGPUSolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params);
    ~BruteforceGPUSolver();

private:
    virtual void ComputeAcceleration() override;
};
