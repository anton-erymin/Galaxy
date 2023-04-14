#pragma once

#pragma once

#include "ISolver.h"

class Thread;
class BarnesHutCPUTree;

class BarnesHutCPUSolver : public ISolver
{
public:
    BarnesHutCPUSolver(Universe& universe);
    ~BarnesHutCPUSolver();

    virtual void Start() override;
    void Solve(float time) override;

    virtual condition_variable* GetSolverConditionVariable() override { return &solver_cv_; }
    virtual void SetPositionsUpdateCompletedFlag(atomic_bool& flag) { positions_update_completed_flag_ = &flag; }

private:
    void TraverseTree(const BarnesHutCPUTree& node);

private:
    vector<mutex> force_mutexes_;
    unique_ptr<Thread> thread_;
    volatile atomic_bool active_flag_;
    condition_variable solver_cv_;
    mutex solver_mu_;
    atomic_bool* positions_update_completed_flag_ = nullptr;

    unique_ptr<BarnesHutCPUTree> tree_;
};
