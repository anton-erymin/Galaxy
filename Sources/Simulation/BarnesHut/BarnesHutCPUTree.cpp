#include "BarnesHutCPUTree.h"
#include "MathUtils.h"
#include "Math/Math.h"

static constexpr uint32_t cMaxTreeLevel = 64;
static constexpr uint32_t cNodesStackSize = 512;
static constexpr float cOpeningAngle = 0.7f;

static size_t lastId = 0;

BarnesHutCPUTree::BarnesHutCPUTree(const float2 &point, float length)
    : point(point),
    length(length)
{
    oppositePoint = point + float2(length);
    id = lastId++;
}

void BarnesHutCPUTree::Reset()
{
    isLeaf = true;
    isBusy = false;
}

void BarnesHutCPUTree::SetBoundingBox(const BoundingBox& bbox)
{
    float max_len = bbox.max_extent();
    float half_max_len = 0.5f * max_len;
    float3 bbox_center = bbox.center();
    float2 bbox_center_2d = float2(bbox_center.x, bbox_center.z);

    point = bbox_center_2d - float2(half_max_len);
    oppositePoint = bbox_center_2d + float2(half_max_len);
    length = max_len;
}

void BarnesHutCPUTree::SetStartPointAndLength(const float2& point, float length)
{
    this->length = length;
    this->point = point;
    oppositePoint = point + float2(length);
}

void BarnesHutCPUTree::Insert(const float2 &position, float bodyMass, uint32_t level)
{
    if (!Contains(position))
    {
        return;
    }

    if (level > cMaxTreeLevel)
    {
        //return;
    }
    
    if (isLeaf)
    {
        if (!isBusy)
        {
            center = position;
            mass = bodyMass;
            isBusy = true;
            return;
        }
        else
        {		
            isLeaf = false;
            ResetChildren();
            
            for (int i = 0; i < 4; i++)
            {
                if (children[i]->Contains(center))
                {
                    children[i]->Insert(center, mass, level + 1);
                    break;
                }
            }

            for (int i = 0; i < 4; i++)
            {
                if (children[i]->Contains(position))
                {
                    children[i]->Insert(position, bodyMass, level + 1);
                    break;
                }
            }

            float totalMass = bodyMass + mass;
            center *= mass;
            center += bodyMass * position;
            center *= (1.0f / totalMass);
            mass = totalMass; 
        }
    }
    else
    {
        float totalMass = bodyMass + mass;
        center *= mass;
        center += bodyMass * position;
        center *= (1.0f / totalMass);
        mass = totalMass;

        for (int i = 0; i < 4; i++)
        {
            if (children[i]->Contains(position))
            {
                children[i]->Insert(position, bodyMass, level + 1);
                break;
            }
        }
    }
}

void BarnesHutCPUTree::InsertFlat(const float2& position, float bodyMass)
{
    BarnesHutCPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    float2 insertedPosition[2] = { position };
    float insertedMass[2] = { bodyMass };
    int8_t current = 0;
    
    while (count)
    {
        BarnesHutCPUTree& node = *stack[--count];

        if (!node.Contains(insertedPosition[current]))
        {
            continue;
        }

        if (node.isLeaf)
        {
            if (!node.isBusy)
            {
                node.center = insertedPosition[current];
                node.mass = insertedMass[current];
                node.isBusy = true;
                if (--current < 0)
                {
                    break;
                }
            }
            else
            {
                node.isLeaf = false;
                node.ResetChildren();

                for (int i = 0; i < 4; i++)
                {
                    if (node.children[i] && node.children[i]->Contains(position))
                    {
                        stack[count++] = node.children[i].get();
                        break;
                    }
                }

                for (int i = 0; i < 4; i++)
                {
                    if (node.children[i] && node.children[i]->Contains(node.center))
                    {
                        ++current;
                        assert(current < 2);
                        insertedPosition[current] = node.center;
                        insertedMass[current] = node.mass;
                        stack[count++] = node.children[i].get();
                        break;
                    }
                }

                float totalMass = bodyMass + node.mass;
                node.center *= node.mass;
                node.center += bodyMass * position;
                node.center *= (1.0f / totalMass);
                node.mass = totalMass;
            }
        }
        else
        {
            float totalMass = bodyMass + node.mass;
            node.center *= node.mass;
            node.center += bodyMass * position;
            node.center *= (1.0f / totalMass);
            node.mass = totalMass;

            for (int i = 0; i < 4; i++)
            {
                if (node.children[i] && node.children[i]->Contains(position))
                {
                    stack[count++] = node.children[i].get();
                    break;
                }
            }
        }
    }
}

inline bool BarnesHutCPUTree::Contains(const float2 &position) const
{
    if (position.x >= point.x && position.x <= oppositePoint.x &&
        position.y >= point.y && position.y <= oppositePoint.y)
        return true;

    return false;
}

inline void BarnesHutCPUTree::ResetChildren()
{
    if (!children[0])
    {
        children[0] = make_unique<BarnesHutCPUTree>(float2(), 0);
        children[1] = make_unique<BarnesHutCPUTree>(float2(), 0);
        children[2] = make_unique<BarnesHutCPUTree>(float2(), 0);
        children[3] = make_unique<BarnesHutCPUTree>(float2(), 0);
    }

    float nl = 0.5f * length;

    float x = point.x + nl;
    float y = point.y + nl;

    float2 np1(x, point.y);
    float2 np2(x, y);
    float2 np3(point.x, y);

    children[0]->SetStartPointAndLength(point, nl);
    children[1]->SetStartPointAndLength(np1, nl);
    children[2]->SetStartPointAndLength(np2, nl);
    children[3]->SetStartPointAndLength(np3, nl);

    children[0]->Reset();
    children[1]->Reset();
    children[2]->Reset();
    children[3]->Reset();
}

size_t BarnesHutCPUTree::GetNodesCount() const
{
    size_t nodesCount = 0;

    const BarnesHutCPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutCPUTree& node = *stack[--count];

        ++nodesCount;

        for (int i = 0; i < 4; i++)
        {
            auto child = node.children[i].get();
            if (child)
            {
                assert(count < cNodesStackSize);
                stack[count++] = child;
            }
        }
    }

    return nodesCount;
}

float2 BarnesHutCPUTree::ComputeAcceleration(const float2& position, float soft) const
{
    float2 acceleration = {};

    if (isLeaf)
    {
        if (isBusy && !equal_eps(position, center, EPS))
        {
            //acceleration = GravityAcceleration(center - position, mass, soft);
        }
    }
    else
    {
        float2 vec = center - position;
        float r = Math::length(vec);
        float theta = length / r;

        if (theta < cOpeningAngle)
        {
            //acceleration = GravityAcceleration(vec, mass, soft, r);
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                if (children[i])
                {
                    acceleration += children[i]->ComputeAcceleration(position, soft);
                }
            }
        }
    }

    return acceleration;
}

float2 BarnesHutCPUTree::ComputeAccelerationFlat(const float2& position, float soft) const
{
    float2 acceleration = {};

    const BarnesHutCPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutCPUTree& node = *stack[--count];

        if (node.isLeaf)
        {
            if (node.isBusy && !equal_eps(position, node.center, EPS))
            {
                //acceleration += GravityAcceleration(node.center - position, node.mass, soft);
            }
        }
        else
        {
            float2 vec = node.center - position;
            float r = Math::length(vec);
            float theta = node.length / r;

            if (theta < cOpeningAngle)
            {
                //acceleration += GravityAcceleration(vec, node.mass, soft, r);
            }
            else
            {
                for (int i = 3; i >= 0; --i)
                {
                    auto child = node.children[i].get();
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
