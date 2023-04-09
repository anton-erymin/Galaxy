#pragma once

#include "ISolver.h"

class Thread;

class BruteforceCPUSolver : public ISolver
{
public:
    BruteforceCPUSolver(Universe& universe);
    ~BruteforceCPUSolver();

    void Solve(float time) override;

private:
    unique_ptr<Thread> thread_;
    volatile atomic_bool active_flag_;
};
