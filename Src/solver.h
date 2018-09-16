#pragma once

#include <vector>

#include "GalaxySystem.h"

class ISolver {
public:
    virtual ~ISolver() = default;
    virtual void solve(float dt, std::vector<GalaxySystem> &galaxies) = 0;
};

class BruteforceSolver : public ISolver {
public:
    void solve(float dt, std::vector<GalaxySystem> &galaxies) override;
};