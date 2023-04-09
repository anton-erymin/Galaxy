#pragma once

class Universe;

class ISolver
{
public:
    ISolver(Universe& universe)
        : universe_(universe)
    {
    }

    virtual ~ISolver() = default;

    virtual void Solve(float time) = 0;
    virtual void SolveForces() { }
    virtual void Inititalize(float time) { }
    virtual void Prepare() { }

protected:
    Universe& universe_;
};
