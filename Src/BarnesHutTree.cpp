#include "BarnesHutTree.h"
#include "Math.h"

#include <cassert>

static constexpr uint32_t cMaxTreeLevel = 64;
static constexpr uint32_t cNodesStackSize = 512;
static constexpr float cOpeningAngle = 0.7f;

static size_t lastId = 0;

BarnesHutTree::BarnesHutTree(const float3 &point, float length)
    : point(point),
    length(length)
{
    oppositePoint = point + float3(length);
    id = lastId++;
}

inline void BarnesHutTree::Reset()
{
    isLeaf = true;
    isBusy = false;
}

void BarnesHutTree::Insert(const float3 &position, float bodyMass, uint32_t level)
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
            center.scale(mass);
            center.addScaled(position, bodyMass);
            center *= (1.0f / totalMass);
            mass = totalMass; 
        }
    }
    else
    {
        float totalMass = bodyMass + mass;
        center.scale(mass);
        center.addScaled(position, bodyMass);
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

void BarnesHutTree::InsertFlat(const float3& position, float bodyMass)
{
    BarnesHutTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    float3 insertedPosition[2] = { position };
    float insertedMass[2] = { bodyMass };
    int8_t current = 0;
    
    while (count)
    {
        BarnesHutTree& node = *stack[--count];

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
                node.center.scale(node.mass);
                node.center.addScaled(position, bodyMass);
                node.center *= (1.0f / totalMass);
                node.mass = totalMass;
            }
        }
        else
        {
            float totalMass = bodyMass + node.mass;
            node.center.scale(node.mass);
            node.center.addScaled(position, bodyMass);
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

inline bool BarnesHutTree::Contains(const float3 &position) const
{
    if (position.m_x >= point.m_x && position.m_x <= oppositePoint.m_x &&
        position.m_y >= point.m_y && position.m_y <= oppositePoint.m_y)
        return true;

    return false;
}

inline void BarnesHutTree::ResetChildren()
{
    if (!children[0])
    {
        float nl = 0.5f * length;

        float x = point.m_x + nl;
        float y = point.m_y + nl;

        float3 np1(x, point.m_y, 0.0f);
        float3 np2(x, y, 0.0f);
        float3 np3(point.m_x, y, 0.0f);

        children[0] = std::make_unique<BarnesHutTree>(point, nl);
        children[1] = std::make_unique<BarnesHutTree>(np1, nl);
        children[2] = std::make_unique<BarnesHutTree>(np2, nl);
        children[3] = std::make_unique<BarnesHutTree>(np3, nl);
    }
    else
    {
        children[0]->Reset();
        children[1]->Reset();
        children[2]->Reset();
        children[3]->Reset();
    }
}

size_t BarnesHutTree::GetNodesCount() const
{
    size_t nodesCount = 0;

    const BarnesHutTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutTree& node = *stack[--count];

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

float3 BarnesHutTree::ComputeAcceleration(const float3& position, float soft) const
{
    float3 acceleration = {};

    if (isLeaf)
    {
        if (isBusy && !equal(position, center, EPS))
        {
            acceleration = GravityAcceleration(center - position, mass, soft);
        }
    }
    else
    {
        float3 vec = center - position;
        float r = vec.norm();
        float theta = length / r;

        if (theta < cOpeningAngle)
        {
            acceleration = GravityAcceleration(vec, mass, soft, r);
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

float3 BarnesHutTree::ComputeAccelerationFlat(const float3& position, float soft) const
{
    float3 acceleration = {};

    const BarnesHutTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutTree& node = *stack[--count];

        if (node.isLeaf)
        {
            if (node.isBusy && !equal(position, node.center, EPS))
            {
                acceleration += GravityAcceleration(node.center - position, node.mass, soft);
            }
        }
        else
        {
            float3 vec = node.center - position;
            float r = vec.norm();
            float theta = node.length / r;

            if (theta < cOpeningAngle)
            {
                acceleration += GravityAcceleration(vec, node.mass, soft, r);
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
