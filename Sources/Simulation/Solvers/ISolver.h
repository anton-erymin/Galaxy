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

    virtual void Start() = 0;
    virtual void Solve(float time) = 0;
    virtual void SolveForces() { }
    virtual void Inititalize(float time) { }
    virtual void Prepare() { }
    virtual condition_variable* GetSolverConditionVariable() { return nullptr; }
    virtual void SetPositionsUpdateCompletedFlag(atomic_bool& flag) { }

protected:
    Universe& universe_;
};
