#pragma once

#include <vector>
#include <memory>

#include "GalaxySystem.h"
#include "BarnesHutTree.h"
#include "solver.h"

class Universe {
public:
    explicit Universe(float size);

    void addGalaxy(const GalaxySystem &galaxy) { galaxies_.push_back(galaxy); }
    void step(float dt);
    void draw(int mode) const;

private:
    std::vector<GalaxySystem> galaxies_;
    BarnesHutTree		      bht_;

    std::unique_ptr<ISolver> solver_;
};

