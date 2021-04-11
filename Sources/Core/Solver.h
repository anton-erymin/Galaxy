#pragma once

#include <memory>
#include <mutex>

class Universe;
class BarnesHutTree;

class Solver {
public:
    Solver(Universe& universe) 
        : universe(universe)
    {
    }

    virtual ~Solver() = default;

    virtual void Solve(float time) = 0;
    virtual void SolveForces() { }
    virtual void Inititalize(float time) { }
    virtual void Prepare() { }

protected:
    Universe& universe;
};

class BruteforceSolver : public Solver {
public:
    BruteforceSolver(Universe& universe) 
        : Solver(universe)
    {
    }

    void Solve(float time) override;
};

class BarnesHutCPUSolver : public Solver
{
public:
    BarnesHutCPUSolver(Universe& universe);

    void Solve(float time) override;
    void SolveForces() override;

    const BarnesHutTree& GetBarnesHutTree() const { return *barnesHutTree; }
    std::mutex& GetTreeMutex() { return mu; }

    void Inititalize(float time) override;

private:
    void BuildTree();

    std::unique_ptr<BarnesHutTree> barnesHutTree;
    std::mutex mu;
};