#include "BarnesHutCPUTree.h"
#include "MathUtils.h"
#include "Math/Math.h"

static constexpr uint32_t cMaxTreeLevel = 64;
static constexpr uint32_t cNodesStackSize = 512;

static size_t lastId = 0;

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

BarnesHutCPUTree::BarnesHutCPUTree()
{
    id_ = lastId++;
}

void BarnesHutCPUTree::Reset()
{
    is_leaf_ = true;
    is_busy_ = false;
}

void BarnesHutCPUTree::SetBoundingBox(const BoundingBox& bbox)
{
    float max_len = bbox.max_extent();
    float half_max_len = 0.5f * max_len;
    float3 bbox_center = bbox.center();
    float2 bbox_center_2d = float2(bbox_center.x, bbox_center.z);

    point_ = bbox_center_2d - float2(half_max_len);
    opposite_point_ = bbox_center_2d + float2(half_max_len);
    length_ = max_len;
}

void BarnesHutCPUTree::SetStartPointAndLength(const float2& point, float length)
{
    length_ = length;
    point_ = point;
    opposite_point_ = point + float2(length);
}

void BarnesHutCPUTree::Insert(const float2 &position, float body_mass, uint32_t level)
{
#if 0
    if (!Contains(position))
    {
        return;
    }
    if (level > cMaxTreeLevel)
    {
        //return;
    }
#endif // 0
    
    bool need_adjust = false;

    if (is_leaf_)
    {
        if (!is_busy_)
        {
            center_ = position;
            mass_ = body_mass;
            is_busy_ = true;
            return;
        }
        else
        {		
            is_leaf_ = false;
            ResetChildren();
            
            for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
            {
                if (children_[i]->Contains(center_))
                {
                    children_[i]->Insert(center_, mass_, level + 1);
                    break;
                }
            }

            for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
            {
                if (children_[i]->Contains(position))
                {
                    children_[i]->Insert(position, body_mass, level + 1);
                    break;
                }
            }

            need_adjust = true;
        }
    }
    else
    {
        need_adjust = true;

        for (int i = 0; i < TREE_CHILDREN_COUNT; i++)
        {
            if (children_[i]->Contains(position))
            {
                children_[i]->Insert(position, body_mass, level + 1);
                break;
            }
        }
    }

    if (need_adjust)
    {
        float totalMass = body_mass + mass_;
        center_ *= mass_;
        center_ += body_mass * position;
        center_ *= (1.0f / totalMass);
        mass_ = totalMass;
    }
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

bool BarnesHutCPUTree::Contains(const float2 &position) const
{
    return 
        (position.x >= point_.x && position.x <= opposite_point_.x &&
        position.y >= point_.y && position.y <= opposite_point_.y);
}

void BarnesHutCPUTree::ResetChildren()
{
    if (!children_[0])
    {
        for (size_t i = 0; i < TREE_CHILDREN_COUNT; i++)
        {
            children_[i] = make_unique<BarnesHutCPUTree>();
        }
    }

    float nl = 0.5f * length_;

    float x = point_.x + nl;
    float y = point_.y + nl;

    float2 np1(x, point_.y);
    float2 np2(x, y);
    float2 np3(point_.x, y);

    children_[0]->SetStartPointAndLength(point_, nl);
    children_[1]->SetStartPointAndLength(np1, nl);
    children_[2]->SetStartPointAndLength(np2, nl);
    children_[3]->SetStartPointAndLength(np3, nl);

    for (size_t i = 0; i < TREE_CHILDREN_COUNT; i++)
    {
        children_[i]->Reset();
    }
}

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
