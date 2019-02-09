#pragma once

#include <memory>

#include "lpVec3.h"
#include "GalaxyParticle.h"

class BarnesHutTree {
public:
    BarnesHutTree(const lpVec3 &point, float length);

    void insert(const GalaxyParticle &p);
    void calcForce(GalaxyParticle &p) const;
    void reset();

    const lpVec3& point() const { return point_; }
    float         length() const { return length_; }
    bool          isLeaf() const { return isLeaf_; }

    const BarnesHutTree& operator[](size_t i) const { return *subs_[i]; }

private:
    bool inline contains(const GalaxyParticle &p) const;

    lpVec3 point_;
    lpVec3 oppositePoint_;
    float  length_;
    float  totalMass_;
    lpVec3 massCenter;
    bool   isLeaf_;

    std::unique_ptr<BarnesHutTree> subs_[4];

    const GalaxyParticle *particle_;
};

void drawTree(const BarnesHutTree &node);
