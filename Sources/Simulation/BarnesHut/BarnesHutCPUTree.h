#pragma once

namespace Math { class BoundingBox; }

constexpr uint32 TREE_CHILDREN_COUNT = 4;

// TODO: Improve by storing nodes in linear array
class BarnesHutCPUTree
{
public:
    BarnesHutCPUTree();

    void Insert(const float2 &position, float body_mass, uint32_t level = 0);
    void InsertFlat(const float2 &position, float body_mass);
    float2 ComputeAcceleration(const float2 &position, float soft, float opening_angle) const;
    float2 ComputeAccelerationFlat(const float2 &position, float soft, float opening_angle) const;
    void Reset();

    void SetBoundingBox(const BoundingBox& bbox);
    void SetStartPointAndLength(const float2& point, float length);

private:
    inline bool Contains(const float2 &position) const;
    inline void ResetChildren();

    // TODO: Replace with BoundingBox
    float2 point_;
    float2 opposite_point_;
    float length_ = 0.0f;
    float mass_ = 0.0f;
    float2 center_;
    bool is_leaf_ = true;
    bool is_busy_ = false;

    unique_ptr<BarnesHutCPUTree> children_[TREE_CHILDREN_COUNT];
    
    size_t id_ = 0;

    friend class BarnesHutCPUSolver;
};
