#pragma once

namespace Math { class BoundingBox; }

class BarnesHutCPUTree
{
public:
    BarnesHutCPUTree();

    void Insert(const float2 &position, float bodyMass, uint32_t level = 0);
    void InsertFlat(const float2 &position, float bodyMass);
    float2 ComputeAcceleration(const float2 &position, float soft) const;
    float2 ComputeAccelerationFlat(const float2 &position, float soft) const;
    void Reset();

    void SetBoundingBox(const BoundingBox& bbox);
    void SetStartPointAndLength(const float2& point, float length);

    const float2& GetPoint() const { return point; }
    float GetLength() const { return length; }
    bool IsLeaf() const { return isLeaf; }

    const BarnesHutCPUTree& operator[](size_t i) const { return *children[i]; }

    size_t GetNodesCount() const;

private:
    inline bool Contains(const float2 &position) const;
    inline void ResetChildren();

    float2 point;
    float2 oppositePoint;
    float  length = 0.0f;
    float  mass = 0.0f;
    float2 center;
    bool   isLeaf = true;
    bool   isBusy = false;

    unique_ptr<BarnesHutCPUTree> children[4];
    
    size_t id = 0;

    friend class BarnesHutCPUSolver;
};
