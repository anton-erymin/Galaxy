#pragma once

#include "ISolver.h"

class Thread;

class BruteforceCPUSolver : public ISolver
{
public:
    BruteforceCPUSolver(Universe& universe, SimulationContext& context);
    ~BruteforceCPUSolver();

    virtual void Start() override;
    void Solve(float time) override;

private:
    vector<mutex> force_mutexes_;
    unique_ptr<Thread> thread_;
    volatile atomic_bool active_flag_;
};
