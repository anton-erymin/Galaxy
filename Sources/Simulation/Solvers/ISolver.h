#pragma once

class Universe;
struct SimulationContext;
struct RenderParameters;

class ISolver
{
public:
    ISolver(Universe& universe, SimulationContext& context, const RenderParameters& render_params)
        : universe_(universe)
        , context_(context)
        , render_params_(render_params)
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
    const RenderParameters& render_params_;
};
