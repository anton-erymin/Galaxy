#pragma once

#include <memory>
#include <mutex>

class Universe;
class BarnesHutTree;





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