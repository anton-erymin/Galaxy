#pragma once

class Universe;
struct SimulationContext;

class ISolver
{
public:
    ISolver(Universe& universe, SimulationContext& context)
        : universe_(universe)
        , context_(context)
    {
    }

    virtual ~ISolver() = default;

    virtual void Start() = 0;
    virtual void Solve(float time) = 0;
    virtual void SolveForces() { }
    virtual void Inititalize(float time) { }
    virtual void Prepare() { }

protected:
    Universe& universe_;
    SimulationContext& context_;
};
