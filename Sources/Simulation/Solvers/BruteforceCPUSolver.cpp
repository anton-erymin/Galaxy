#include "BruteforceCPUSolver.h"
#include "Universe.h"
#include "Galaxy.h"
#include "MathUtils.h"

#include <Thread/Thread.h>

BruteforceCPUSolver::BruteforceCPUSolver(Universe& universe)
    : ISolver(universe)
{
    active_flag_ = true;

    thread_.reset(new Thread("BruteforceCPUSolver Thread",
        [this]()
        {
            while (active_flag_)
            {
                Solve(0.0000001f);
            }
        }));
}

BruteforceCPUSolver::~BruteforceCPUSolver()
{
    active_flag_ = false;
}

void BruteforceCPUSolver::Solve(float time)
{
    // Bruteforce

    size_t count = universe_.positions_.size();

    for (size_t i = 0; i < count; i++)
    {
        for (size_t j = i + 1; j < count; j++)
        {
            float3 l = universe_.positions_[j] - universe_.positions_[i];
            float dist = SoftenedDistance(l.length_sq(), cSoftFactor);
            float dist_cubic = dist * dist * dist;

            float3 force1 = GravityAcceleration(l, universe_.masses_[j], dist_cubic);
            float3 force2 = GravityAcceleration(-l, universe_.masses_[i], dist_cubic);

            universe_.forces_[i] += force1;
            universe_.forces_[j] += force2;
        }
    }

    for (size_t i = 0; i < count; i++)
    {
        if (universe_.masses_[i] > 0.0f)
        {
            IntegrateMotionEquation(time, universe_.positions_[i], universe_.velocities_[i], 
                universe_.accelerations_[i], universe_.forces_[i], universe_.inverse_masses_[i]);
        }

        // Clear force accumulator
        universe_.forces_[i] = float3();
    }
}
