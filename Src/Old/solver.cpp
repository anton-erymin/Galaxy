#include "solver.h"

void BruteforceSolver::solve(float dt, std::vector<GalaxySystem> &galaxies) {
    // Bruteforce
    size_t gi = 0;
    for (auto &g1 : galaxies) {
        size_t pi = 0;
        for (auto &p1 : g1.particles()) {
            for (size_t pj = pi + 1; pj < g1.particles().size(); ++pj) {
                auto &p2 = g1.particles()[pj];
                auto force = p1.caclGravityForce(p2);

                p1.m_forceAccum += force;
                p2.m_forceAccum -= force;
            }

            for (size_t gj = gi + 1; gj < galaxies.size(); ++gj) {
                for (auto &p2 : galaxies[gj].particles()) {
                    auto force = p1.caclGravityForce(p2);

                    p1.m_forceAccum += force;
                    p2.m_forceAccum -= force;
                }
            }
            ++pi;
        }
        ++gi;
    }
}