#include "BarnesHutCPUTree.h"
#include "MathUtils.h"
#include "Math/Math.h"

#include <Thread/ThreadPool.h>

static constexpr uint32_t cMaxTreeLevel = 64;
static constexpr uint32_t cNodesStackSize = 64;

BarnesHutCPUTree::BarnesHutCPUTree(const vector<float4>& body_position, const vector<float>& body_mass)
    : body_position_(body_position)
    , body_mass_(body_mass)
    , mass_(2 * body_position.size())
    , children_mu_(2 * body_position.size() * TREE_CHILDREN_COUNT)
{
    size_t nodes_max_count = 2 * GetBodyCount();
    position_.resize(nodes_max_count);
    children_.resize(TREE_CHILDREN_COUNT * nodes_max_count);
    bbox_per_thread_.resize(ThreadPool::GetThreadCount());
}

void BarnesHutCPUTree::BuildTree()
{
    BoundingBox bbox = ComputeBoundingBox();
    ResetTree(bbox);
    BuildHierarchy();
}

BoundingBox BarnesHutCPUTree::ComputeBoundingBox()
{
    auto BoundingBoxKernel = [this](THREAD_POOL_KERNEL_ARGS)
    {
        bbox_per_thread_[block_id].grow(body_position_[global_id]);
    };

    PARALLEL_FOR(GetBodyCount(), BoundingBoxKernel);

    // Compute final bounding box
    BoundingBox bbox;
    for (size_t i = 0; i < bbox_per_thread_.size(); i++)
    {
        bbox.grow(bbox_per_thread_[i]);
        // Reset box
        bbox_per_thread_[i] = BoundingBox();
    }

    return bbox;
}

void BarnesHutCPUTree::ResetTree(const BoundingBox& bbox)
{
    int max_dim_idx = bbox.maxdim();
    float max_len = bbox.max_extent();
    float3 bbox_center = bbox.center();

    radius_ = 0.5f * max_len;
    cur_node_idx_ = GetRootNode();

    // Setup root node
    AddNode(bbox_center);
}

void BarnesHutCPUTree::BuildHierarchy()
{
    auto BuildHierarchyKernel = [this](THREAD_POOL_KERNEL_ARGS)
    {
        InsertBody(global_id);
    };

    PARALLEL_FOR(GetBodyCount(), BuildHierarchyKernel);
}

static int32 FindChildBranch(const float4& node_center, const float4& body_pos)
{
    int32 branch = 0;
    if (body_pos.x > node_center.x) branch = 1;
    if (body_pos.y > node_center.y) branch += 2;
    if (body_pos.z > node_center.z) branch += 4;
    return branch;
}

float4 BarnesHutCPUTree::GetChildCenterPos(const float4& node_center, int32 child_branch, float radius)
{
    float half_radius = 0.5f * radius;
    float4 child_offset = 
        float4((child_branch & 1) * radius, 
            ((child_branch >> 1) & 1) * radius,
            ((child_branch >> 2) & 1) * radius, 
            0.0f);

    // 0 (0, 0)
    // 1 (radius, 0)
    // 2 (0, radius)
    // 3 (radius, radius)

    float4 pos = node_center - float4(half_radius) + child_offset;
    return pos;
}

void BarnesHutCPUTree::InsertBody(int32 body)
{
    const float4& body_pos = body_position_[body];

    // Setup following from root
    int32 node = GetRootNode();
    float4 node_pos = GetPosition(node);
    float current_radius = radius_;

    int32 subdivided_node, subdivided_branch, subtree, old_child_branch;

    int32 child_branch = FindChildBranch(node_pos, body_pos);

    LockChild(node, child_branch);
    int32 child_index = GetChildIndex(node, child_branch);

    // Follow from root to the current available leaf
    while (IsNode(child_index))
    {
        UnlockChild(node, child_branch);
        node = child_index;
        node_pos = GetPosition(node);
        child_branch = FindChildBranch(node_pos, body_pos);

        LockChild(node, child_branch);
        child_index = GetChildIndex(node, child_branch);
        current_radius *= 0.5f;
    }

    if (IsNull(child_index))
    {
        // No body here yet so just insert and unlock
        SetChildIndex(node, child_branch, body);
        UnlockChild(node, child_branch);
    }
    else
    {
        // Child is body
        // Create new node(s) and insert the old and new body
        assert(IsBody(child_index));

        const float4& old_body_pos = GetPosition(child_index);

        subdivided_node = node;
        subdivided_branch = child_branch;

        subtree = -1;

        do
        {
            // Create new node
            int32 new_node = AddNode(GetChildCenterPos(node_pos, child_branch, current_radius));
            const float4& new_node_pos = GetPosition(new_node);

            // Get index of subtree
            subtree = max(subtree, new_node);

            // Link new child node to parent
            if (new_node != subtree)
            {
                SetChildIndex(node, child_branch, new_node);
            }

            // Find branches for old and new bodies in newly created node
            child_branch = FindChildBranch(new_node_pos, body_pos);
            old_child_branch = FindChildBranch(new_node_pos, old_body_pos);

            node = new_node;
            node_pos = new_node_pos;

            current_radius *= 0.5f;
        }
        while (child_branch == old_child_branch); // Subdivide until two bodies are in different child branches

        SetChildIndex(node, old_child_branch, child_index);
        SetChildIndex(node, child_branch, body);
        // Attach new subtree to tree
        SetChildIndex(subdivided_node, subdivided_branch, subtree);
        UnlockChild(subdivided_node, subdivided_branch);
    }
}

int32 BarnesHutCPUTree::AddNode(const float4& node_center_pos)
{
    int32 new_node = cur_node_idx_--;
    if (!IsNode(new_node))
    {
        NLOG_FATAL("BarnesHutCPUTree: No space left for nodes");
    }

    // Initialize new node
    SetPosition(new_node, node_center_pos);
    SetMass(new_node, -1.0f);
    for (size_t i = 0; i < TREE_CHILDREN_COUNT; i++)
    {
        SetChildIndex(new_node, i, NULL_INDEX);
    }
    return new_node;
}

void BarnesHutCPUTree::SummarizeTree()
{
    auto SummarizeKernel = [this](THREAD_POOL_KERNEL_ARGS)
    {
        int32 node = cur_node_idx_ + global_id + 1;

        float node_mass = GetMass(node);
        assert(IsNode(node) && node_mass == -1.0f);

        float4 gravity_center = float4();
        node_mass = 0.0f;

        // Counter of non-completed nodes (mass is not available yet)
        size_t missing_count = 0;
        int32 missing_childs[TREE_CHILDREN_COUNT];

        for (size_t j = 0; j < TREE_CHILDREN_COUNT; j++)
        {
            int32 child_index = GetChildIndex(node, j);
            bool is_child_node = IsNode(child_index);

            if (IsBody(child_index) || is_child_node)
            {
                float child_mass = GetMass(child_index);

                if (is_child_node && child_mass < 0.0f)
                {
                    // Add to missing
                    missing_childs[missing_count++] = child_index;
                    continue;
                }

                // Add contribution
                assert(child_mass >= 0.0f);
                gravity_center += child_mass * GetPosition(child_index);
                node_mass += child_mass;
            }
        }

        int32 missing_child_i = 0;
        while (missing_count > 0)
        {
            int32 child_index = missing_childs[missing_child_i];
            float child_mass = GetMass(child_index);

            if (child_mass < 0.0f)
            {
                missing_child_i = (missing_child_i + 1) % missing_count;
                continue;
            }

            // Add contribution
            gravity_center += child_mass * GetPosition(child_index);
            node_mass += child_mass;

            int32 last = missing_count - 1;
            if (missing_child_i != last)
            {
                // Remove current missing
                swap(missing_childs[missing_child_i], missing_childs[last]);
            }
            else
            {
                missing_child_i = 0;
            }

            --missing_count;
        }

        assert(missing_count == 0);

        gravity_center *= (1.0f / node_mass);

        SetPosition(node, gravity_center);
        // Mass is atomically written last
        SetMass(node, node_mass);
    };

    ThreadPool::Dispatch(GetActualNodeCount(), 1, SummarizeKernel);
}

float4 BarnesHutCPUTree::ComputeAcceleration(int32 body, float soft, float opening_angle) const
{
    float4 acceleration = {};

    const float4& body_position = GetPosition(body);

    int32 stack[cNodesStackSize];
    float coeff_stack[cNodesStackSize];

    stack[0] = GetRootNode();
    coeff_stack[0] = 4.0f * radius_ * radius_ / (opening_angle * opening_angle);

    size_t stack_count = 1;

    while (stack_count)
    {
        --stack_count;
        int32 node = stack[stack_count];
        float threshold_dist = coeff_stack[stack_count];

        const float4& center = GetPosition(node);
        float mass = GetMass(node);

        float3 l = center - body_position;
        float dist_sq = l.sqnorm();

        if (IsBody(node) || dist_sq > threshold_dist)
        {
            float dist_soft = SoftenedDistance(dist_sq, soft);
            float dist_soft_cubic = dist_soft * dist_soft * dist_soft;
            acceleration += GravityAcceleration(l, mass, dist_soft_cubic);
        }
        else
        {
            float half_threshold = 0.25f * threshold_dist;

            for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
            {
                int32 child_index = GetChildIndex(node, i);
                if (!IsNull(child_index) && child_index != body)
                {
                    assert(stack_count < cNodesStackSize);
                    stack[stack_count] = child_index;
                    coeff_stack[stack_count] = half_threshold;
                    ++stack_count;
                }
            }
        }
    }

    return acceleration;
}
