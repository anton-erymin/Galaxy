#include "Universe.h"

#include <Windows.h>

#include "constants.h"
#include "texture.h"

TextureImage starTexture;

Universe::Universe(float size) 
    : bht_(lpVec3{ -size * 0.5f }, size) {
    srand(GetTickCount());

    solver_ = std::make_unique<BruteforceSolver>();
}

void Universe::step(float dt) {
    solver_->solve(dt, galaxies_);
}

void Universe::draw(int mode) const {
    for (const auto& x : galaxies_) {
        x.draw(mode);
    }
    if (mode & GLX_RENDER_TREE) {
        drawTree(bht_);
    }
}