#pragma once

#include <memory>

#include "float3.h"

struct Particle;

class BarnesHutTree
{
public:
    BarnesHutTree(const float3 &point, float length);

    void Insert(const Particle &p, uint32_t level = 0);
    float3 ComputeAcceleration(const Particle &particle, float soft) const;
    void Reset();

    const float3& GetPoint() const { return point; }
    float GetLength() const { return length; }
    bool IsLeaf() const { return isLeaf; }

    const BarnesHutTree& operator[](size_t i) const { return *children[i]; }

private:
    bool inline Contains(const Particle &p) const;

    float3 point;
    float3 oppositePoint;
    float  length;
    float  totalMass;
    float3 massCenter;
    bool   isLeaf;

    std::unique_ptr<BarnesHutTree> children[4];

    const Particle *particle_ = nullptr;

    friend void DrawBarnesHutTree(const BarnesHutTree& node);
};
