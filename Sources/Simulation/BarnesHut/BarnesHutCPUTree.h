#pragma once

namespace Math { class AABB; }

constexpr uint32 TREE_CHILDREN_COUNT = 8;
constexpr int32 NULL_INDEX = -1;

class BarnesHutCPUTree
{
public:
    BarnesHutCPUTree(const Array<Float4>& body_position, const Array<float>& body_mass);

    void BuildTree();
    void SummarizeTree();
    Float4 ComputeAcceleration(int32 body, float soft, float opening_angle) const;

private:
    AABB ComputeBoundingBox();
    void ResetTree(const AABB& bbox);
    void BuildHierarchy();
    void InsertBody(int32 body);
    int32 AddNode(const Float4& node_center_pos);
    Float4 GetChildCenterPos(const Float4& node_center, int32 child_branch, float radius);

    // Index helpers
    int32 GetBodyCount() const { return int32(body_position_.Size()); }
    int32 GetNodeMaxCount() const { return int32(position_.Size()); }
    int32 GetActualNodeCount() const { return GetRootNode() - cur_node_idx_; }
    bool IsNull(int32 index) const { return index == NULL_INDEX; }
    bool IsBody(int32 index) const { return !IsNull(index) && index < GetBodyCount(); }
    bool IsBodyOrNull(int32 index) const { return index < GetBodyCount(); }
    bool IsNode(int32 index) const { return index >= GetBodyCount(); }
    int32 GetNodeUniIndex(int32 array_index) const { return array_index + GetBodyCount(); }
    int32 GetNodeArrayIndex(int32 uni_index) const { return uni_index - GetBodyCount(); }
    const Float4& GetPosition(int32 uni_index) const { return IsBody(uni_index) ? body_position_[uni_index] : position_[GetNodeArrayIndex(uni_index)]; }
    void SetPosition(int32 uni_index, const Float4& pos) { NASSERT(IsNode(uni_index)); position_[GetNodeArrayIndex(uni_index)] = pos; }
    float GetMass(int32 uni_index) const { return IsBody(uni_index) ? body_mass_[uni_index] : *mass_[GetNodeArrayIndex(uni_index)]; }
    void SetMass(int32 uni_index, float mass) { NASSERT(IsNode(uni_index)); *mass_[GetNodeArrayIndex(uni_index)] = mass; }
    int32 GetChildIndex(int32 uni_index, int32 child_branch) const { NASSERT(IsNode(uni_index)); return children_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch]; }
    void SetChildIndex(int32 uni_index, int32 child_branch, int32 child_index) { NASSERT(IsNode(uni_index)); children_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch] = child_index; }
    int32 GetRootNode() const { return GetNodeUniIndex(GetNodeMaxCount() - 1); }
    void LockChild(int32 uni_index, int32 child_branch) { NASSERT(IsNode(uni_index)); return children_mu_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch]->lock(); }
    void UnlockChild(int32 uni_index, int32 child_branch) { NASSERT(IsNode(uni_index)); return children_mu_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch]->unlock(); }

private:
    const Array<Float4>& body_position_;
    const Array<float>& body_mass_;

    // Nodes
    // Firstly contain geometrical centers of nodes, after summarize contain centers of gravity
    Array<Float4> position_;
    Array<UniquePtr<Atomic<float>>> mass_;
    Array<int32> children_;
    Array<UniquePtr<Mutex>> children_mu_;

    Array<AABB> bbox_per_thread_;

    // Root radius
    float radius_;

    Atomic<size_t> cur_node_idx_;

    friend class BarnesHutCPUSolver;
};
