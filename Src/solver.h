#pragma once

class Universe;

class Solver {
public:
    virtual ~Solver() = default;

    virtual void Solve(float dt, Universe& universe) = 0;
};

class BruteforceSolver : public Solver {
public:
    void Solve(float dt, Universe& universe) override;
};

class BarnesHutSolver : public Solver
{
public:
    void Solve(float dt, Universe& universe) override;
};