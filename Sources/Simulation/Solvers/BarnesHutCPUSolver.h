#pragma once

#pragma once

#include "ISolver.h"

class Thread;
class BarnesHutCPUTree;

class BarnesHutCPUSolver : public ISolver
{
public:
    BarnesHutCPUSolver(Universe& universe, SimulationContext& context);
    ~BarnesHutCPUSolver();

    virtual void Start() override;
    void Solve(float time) override;

private:
    void TraverseTree(const BarnesHutCPUTree& node);

private:
    // Must be declared before thread
    vector<mutex> force_mutexes_;
    unique_ptr<Thread> thread_;
    volatile atomic_bool active_flag_;

    unique_ptr<BarnesHutCPUTree> tree_;
};
