#pragma once

#include <memory>

#include "lpVec3.h"

struct Particle;

class BarnesHutTree
{
public:
    BarnesHutTree(const lpVec3 &point, float length);

    void Insert(const Particle &p, uint32_t level = 0);
    lpVec3 CalculateForce(const Particle &particle) const;
    void Reset();

    const lpVec3& GetPoint() const { return point; }
    float GetLength() const { return length; }
    bool IsLeaf() const { return isLeaf; }

    const BarnesHutTree& operator[](size_t i) const { return *children[i]; }

private:
    bool inline Contains(const Particle &p) const;

    lpVec3 point;
    lpVec3 oppositePoint;
    float  length;
    float  totalMass;
    lpVec3 massCenter;
    bool   isLeaf;

    std::unique_ptr<BarnesHutTree> children[4];

    const Particle *particle_ = nullptr;

    friend void DrawBarnesHutTree(const BarnesHutTree& node);
};
