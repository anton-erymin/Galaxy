#pragma once

namespace Math { class BoundingBox; }

constexpr uint32 TREE_CHILDREN_COUNT = 4;
constexpr int32 NULL_INDEX = -1;

class BarnesHutCPUTree
{
public:
    BarnesHutCPUTree(const vector<float4>& body_position, const vector<float>& body_mass);

    void BuildTree();
    void SummarizeTree();
    float2 ComputeAcceleration(int32 body, float soft, float opening_angle) const;
    //float2 ComputeAccelerationFlat(const float2 &position, float soft, float opening_angle) const;

private:
    BoundingBox ComputeBoundingBox();
    void ResetTree(const BoundingBox& bbox);
    void BuildHierarchy();
    void InsertBody(int32 body, int32 node, float radius);
    int32 AddNode(const float4& node_center_pos);
    float4 GetChildCenterPos(const float4& node_center, int32 child_branch, float radius);
    float2 ComputeAccelerationRecursive(int32 body, int32 node, float radius, float soft, float opening_angle) const;

    // Index helpers
    int32 GetBodyCount() const { return int32(body_position_.size()); }
    int32 GetNodeCount() const { return int32(position_.size()); }
    int32 GetActualNodeCount() const { return GetRootIndex() - cur_node_idx_; }
    bool IsNull(int32 index) const { return index == NULL_INDEX; }
    bool IsBody(int32 index) const { return !IsNull(index) && index < GetBodyCount(); }
    bool IsBodyOrNull(int32 index) const { return index < GetBodyCount(); }
    bool IsNode(int32 index) const { return index >= GetBodyCount(); }
    int32 GetNodeUniIndex(int32 array_index) const { return array_index + GetBodyCount(); }
    int32 GetNodeArrayIndex(int32 uni_index) const { return uni_index - GetBodyCount(); }
    const float4& GetPosition(int32 uni_index) const { return IsBody(uni_index) ? body_position_[uni_index] : position_[GetNodeArrayIndex(uni_index)]; }
    void SetPosition(int32 uni_index, const float4& pos) { assert(IsNode(uni_index)); position_[GetNodeArrayIndex(uni_index)] = pos; }
    float GetMass(int32 uni_index) const { return IsBody(uni_index) ? body_mass_[uni_index] : mass_[GetNodeArrayIndex(uni_index)]; }
    void SetMass(int32 uni_index, float mass) { assert(IsNode(uni_index)); mass_[GetNodeArrayIndex(uni_index)] = mass; }
    int32 GetChildIndex(int32 uni_index, int32 child_branch) const { assert(IsNode(uni_index)); return children_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch]; }
    void SetChildIndex(int32 uni_index, int32 child_branch, int32 child_index) { assert(IsNode(uni_index)); children_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch] = child_index; }
    int32 GetRootIndex() const { return GetNodeUniIndex(GetNodeCount() - 1); }
    void LockChild(int32 uni_index, int32 child_branch) { assert(IsNode(uni_index)); return children_mu_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch].lock(); }
    void UnlockChild(int32 uni_index, int32 child_branch) { assert(IsNode(uni_index)); return children_mu_[GetNodeArrayIndex(uni_index) * TREE_CHILDREN_COUNT + child_branch].unlock(); }

private:
    const vector<float4>& body_position_;
    const vector<float>& body_mass_;

    // Nodes
    // Firstly contain geometrical centers of nodes, after summarize contain centers of gravity
    vector<float4> position_;
    vector<atomic<float>> mass_;
    vector<int32> children_;
    vector<mutex> children_mu_;

    vector<BoundingBox> bbox_per_thread_;

    // Root radius
    float radius_;

    atomic<size_t> cur_node_idx_;

    friend class BarnesHutCPUSolver;
};
