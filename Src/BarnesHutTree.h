#pragma once

#include <memory>

#include "float3.h"

class BarnesHutTree
{
public:
    BarnesHutTree(const float3 &point, float length);

    void Insert(const float3 &position, float bodyMass, uint32_t level = 0);
    float3 ComputeAcceleration(const float3 &position, float soft) const;
    float3 ComputeAccelerationFlat(const float3 &position, float soft) const;
    void Reset();

    const float3& GetPoint() const { return point; }
    float GetLength() const { return length; }
    bool IsLeaf() const { return isLeaf; }

    const BarnesHutTree& operator[](size_t i) const { return *children[i]; }

private:
    bool inline Contains(const float3 &position) const;

    float3 point;
    float3 oppositePoint;
    float  length;
    float  mass = 0.0f;
    float3 center = {};
    bool   isLeaf = true;
    bool   isBusy = false;

    std::unique_ptr<BarnesHutTree> children[4];
    
    friend void DrawBarnesHutTree(const BarnesHutTree& node);
};
