#include "BarnesHutCPUTree.h"
#include "MathUtils.h"
#include "Math/Math.h"

#include <Thread/ThreadPool.h>

static constexpr uint32_t cMaxTreeLevel = 64;
static constexpr uint32_t cNodesStackSize = 512;

static float2 Float3To2(const float3& v) { return float2(v.x, v.z); }
static float3 Float2To3(const float2& v) { return float3(v.x, 0.0f, v.y); }
static float2 GravityAcceleration2(const float2& l, float mass, float soft)
{
    // TODO: Softended distance right?
    float3 l3 = Float2To3(l);
    float dist = SoftenedDistance(l3.length_sq(), soft);
    float dist_cubic = dist * dist * dist;
    float3 a = GravityAcceleration(l3, mass, dist_cubic);
    return Float3To2(a);
}

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
        bbox_per_thread_[block_id].grow(float3(body_position_[global_id]));
    };

    PARALLEL_FOR(GetBodyCount(), BoundingBoxKernel);

    // Compute final bounding box
    BoundingBox bbox;
    for (size_t i = 0; i < bbox_per_thread_.size(); i++)
    {
        bbox.grow(bbox_per_thread_[i]);
    }

    return bbox;
}

void BarnesHutCPUTree::ResetTree(const BoundingBox& bbox)
{
    int max_dim_idx = bbox.maxdim();
    float max_len = bbox.max_extent();
    float3 bbox_center = bbox.center();

    radius_ = 0.5f * max_len;
    cur_node_idx_ = GetRootIndex();

    // Setup root node
    AddNode(bbox_center);
}

void BarnesHutCPUTree::BuildHierarchy()
{
    auto BuildHierarchyKernel = [this](THREAD_POOL_KERNEL_ARGS)
    {
        InsertBody(global_id, GetRootIndex(), radius_);
    };

    PARALLEL_FOR(GetBodyCount(), BuildHierarchyKernel);
}

static int32 FindChildBranch(const float4& node_center, const float4& body_pos)
{
    // TODO: Fix for octree
    int32 branch = 0;
    if (body_pos.x > node_center.x) branch = 1;
    if (body_pos.z > node_center.z) branch += 2;
    return branch;
}

float4 BarnesHutCPUTree::GetChildCenterPos(const float4& node_center, int32 child_branch, float radius)
{
    // TODO: Fix for octree
    float half_radius = 0.5f * radius;
    float4 child_offset = 
        float4((child_branch & 1) * radius, 0.0f, ((child_branch >> 1) & 1) * radius, 0.0f);

    // 0 (0, 0)
    // 1 (radius, 0)
    // 2 (0, radius)
    // 3 (radius, radius)

    float4 pos = node_center - float4(half_radius) + child_offset;
    pos.y = 0.0f;
    return pos;
}

void BarnesHutCPUTree::InsertBody(int32 body, int32 node, float radius)
{
    const float4& body_pos = body_position_[body];
    const float4& node_pos = GetPosition(node);
    float half_radius = 0.5f * radius;

    int32 branch = FindChildBranch(node_pos, body_pos);

    LockChild(node, branch);
    int32 child_index = GetChildIndex(node, branch);

    if (IsNull(child_index))
    {
        // No body here yet so just insert
        //LockChild(node, branch);
        SetChildIndex(node, branch, body);
        UnlockChild(node, branch);
    }
    else if (IsNode(child_index))
    {
        // Unlock and follow this child
        UnlockChild(node, branch);
        InsertBody(body, child_index, half_radius);
    }
    else
    {
        // Child is body
        // Create new node(s) and insert the old and new body

        //LockChild(node, branch);

        // Setup new node
        int32 new_node = AddNode(GetChildCenterPos(node_pos, branch, radius));

        InsertBody(child_index, new_node, half_radius);
        InsertBody(body, new_node, half_radius);

        // Attach new subtree to tree
        SetChildIndex(node, branch, new_node);

        UnlockChild(node, branch);
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

float2 BarnesHutCPUTree::ComputeAcceleration(int32 body, float soft, float opening_angle) const
{
    return ComputeAccelerationRecursive(body, GetRootIndex(), radius_, soft, opening_angle);
}

float2 BarnesHutCPUTree::ComputeAccelerationRecursive(int32 body, int32 node, float radius, float soft, float opening_angle) const
{
    float2 acceleration = {};

    float2 position = Float3To2(GetPosition(body));
    float2 center = Float3To2(GetPosition(node));
    float mass = GetMass(node);

    float2 l = center - position;

    if (IsBody(node))
    {
        if (body != node)
        {
            acceleration = GravityAcceleration2(l, mass, soft);
        }
    }
    else
    {
        float r = l.length();
        float theta = (2.0f * radius) / r;

        if (theta < opening_angle)
        {
            acceleration = GravityAcceleration2(l, mass, soft);
        }
        else
        {
            for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
            {
                int32 child_index = GetChildIndex(node, i);
                if (!IsNull(child_index))
                {
                    acceleration += ComputeAccelerationRecursive(body, child_index, 0.5f * radius, soft, opening_angle);
                }
            }
        }
    }

    return acceleration;
}

#if 0
float2 BarnesHutCPUTree::ComputeAccelerationFlat(const float2& position, float soft, float opening_angle) const
{
    float2 acceleration = {};

    const BarnesHutCPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutCPUTree& node = *stack[--count];

        if (node.is_leaf_)
        {
            if (node.is_busy_ && !equal_eps(position, node.center, EPS))
            {
                //acceleration += GravityAcceleration(node.center_ - position, node.mass_, soft);
            }
        }
        else
        {
            float2 vec = node.center_ - position;
            float r = vec.length();
            float theta = node.length_ / r;

            if (theta < opening_angle)
            {
                //acceleration += GravityAcceleration(vec, node.mass_, soft, r);
            }
            else
            {
                for (int i = 3; i >= 0; --i)
                {
                    auto child = node.children_[i].get();
                    if (child)
                    {
                        assert(count < cNodesStackSize);
                        stack[count++] = child;
                    }
                }
            }
        }
    }

    return acceleration;
}

void BarnesHutCPUTree::InsertFlat(const float2& position, float body_mass)
{
    BarnesHutCPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    float2 insertedPosition[2] = { position };
    float insertedMass[2] = { body_mass };
    int8_t current = 0;

    while (count)
    {
        BarnesHutCPUTree& node = *stack[--count];

        if (!node.Contains(insertedPosition[current]))
        {
            continue;
        }

        if (node.is_leaf_)
        {
            if (!node.is_busy_)
            {
                node.center_ = insertedPosition[current];
                node.mass_ = insertedMass[current];
                node.is_busy_ = true;
                if (--current < 0)
                {
                    break;
                }
            }
            else
            {
                node.is_leaf_ = false;
                node.ResetChildren();

                for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
                {
                    if (node.children_[i] && node.children_[i]->Contains(position))
                    {
                        stack[count++] = node.children_[i].get();
                        break;
                    }
                }

                for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
                {
                    if (node.children_[i] && node.children_[i]->Contains(node.center_))
                    {
                        ++current;
                        assert(current < 2);
                        insertedPosition[current] = node.center_;
                        insertedMass[current] = node.mass_;
                        stack[count++] = node.children_[i].get();
                        break;
                    }
                }

                float totalMass = body_mass + node.mass_;
                node.center_ *= node.mass_;
                node.center_ += body_mass * position;
                node.center_ *= (1.0f / totalMass);
                node.mass_ = totalMass;
            }
        }
        else
        {
            float totalMass = body_mass + node.mass_;
            node.center_ *= node.mass_;
            node.center_ += body_mass * position;
            node.center_ *= (1.0f / totalMass);
            node.mass_ = totalMass;

            for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
            {
                if (node.children_[i] && node.children_[i]->Contains(position))
                {
                    stack[count++] = node.children_[i].get();
                    break;
                }
            }
        }
    }
}
#endif // 0