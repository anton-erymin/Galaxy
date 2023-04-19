#include "BarnesHutCPUTree.h"
#include "MathUtils.h"
#include "Math/Math.h"

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
{
    size_t nodes_max_count = 2 * GetBodyCount();
    position_.resize(nodes_max_count);
    mass_.resize(nodes_max_count);
    children_.resize(TREE_CHILDREN_COUNT * nodes_max_count);
}

void BarnesHutCPUTree::BuildTree()
{
    for (size_t i = 0; i < GetBodyCount(); i++)
    {
        InsertBody(i, GetRootIndex(), radius_);
    }
}

void BarnesHutCPUTree::SetBoundingBox(const BoundingBox& bbox)
{
    int max_dim_idx = bbox.maxdim();
    float max_len = bbox.max_extent();
    float3 bbox_center = bbox.center();
    //float2 bbox_center_2d = float2(bbox_center.x, bbox_center.z);

    radius_ = 0.5f * max_len;
    cur_node_idx_ = GetRootIndex();

    // Setup root node
    AddNode(bbox_center);
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

float4 BarnesHutCPUTree::GetCenterOfGravity(
    float mass0, const float4& pos0, 
    float mass1, const float4& pos1, float& out_mass)
{
    out_mass = mass0 + mass1;
    return (mass0 * pos0 + mass1 * pos1) * (1.0f / out_mass);
}

void BarnesHutCPUTree::InsertBody(int32 body, int32 node, float radius)
{
    const float4& body_pos = body_position_[body];
    const float4& node_pos = GetPosition(node);
    float half_radius = 0.5f * radius;

    int32 branch = FindChildBranch(node_pos, body_pos);
    int32 child_index = GetChildIndex(node, branch);

    if (child_index == NULL_INDEX)
    {
        // No body here yet so just insert
        SetChildIndex(node, branch, body);
    }
    else if (IsNode(child_index))
    {
        // Follow this child
        InsertBody(body, child_index, half_radius);
    }
    else
    {
        // Child is body
        // Create new cell(s) and insert the old and new body

        // Setup new node
        int32 new_node = AddNode(GetChildCenterPos(node_pos, branch, radius));

        InsertBody(child_index, new_node, half_radius);
        InsertBody(body, new_node, half_radius);

        // Attach new subtree to tree
        SetChildIndex(node, branch, new_node);
    }
}

bool BarnesHutCPUTree::Contains(const float2 &position) const
{
    return false;
#if 0
    return
        (position.x >= point_.x && position.x <= opposite_point_.x &&
            position.y >= point_.y && position.y <= opposite_point_.y);
#endif // 0

}

int32 BarnesHutCPUTree::AddNode(const float4& node_center_pos)
{
    int32 new_node = cur_node_idx_--;
    if (!IsNode(new_node))
    {
        NLOG_FATAL("BarnesHutCPUTree: No space left for nodes");
    }

    SetPosition(new_node, node_center_pos);
    SetMass(new_node, -1.0f);
    for (size_t i = 0; i < TREE_CHILDREN_COUNT; i++)
    {
        SetChildIndex(new_node, i, NULL_INDEX);
    }
    return new_node;
}

#if 0
float2 BarnesHutCPUTree::ComputeAcceleration(const float2& position, float soft, float opening_angle) const
{
    float2 acceleration = {};

    float2 l = center_ - position;

    if (is_leaf_)
    {
        if (is_busy_ && !equal_eps(position, center_, EPS))
        {
            acceleration = GravityAcceleration2(l, mass_, soft);
        }
    }
    else
    {
        float r = l.length();
        float theta = length_ / r;

        if (theta < opening_angle)
        {
            acceleration = GravityAcceleration2(l, mass_, soft);
        }
        else
        {
            for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
            {
                if (children_[i])
                {
                    acceleration += children_[i]->ComputeAcceleration(position, soft, opening_angle);
                }
            }
        }
    }

    return acceleration;
}

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
            if (node.is_busy_ && !equal_eps(position, node.center_, EPS))
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

#endif // 0

#if 0
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