#include "BarnesHutGPUTree.h"
#include "MathUtils.h"

static constexpr uint32_t cMaxTreeLevel = 64;
static constexpr uint32_t cNodesStackSize = 512;
static constexpr float cOpeningAngle = 0.7f;

static size_t lastId = 0;

BarnesHutGPUTree::BarnesHutGPUTree(const float3 &point, float length)
    : point(point),
    length(length)
{
    oppositePoint = point + float3(length);
    id = lastId++;
}

void BarnesHutGPUTree::Reset()
{
    isLeaf = true;
    isBusy = false;
}

void BarnesHutGPUTree::Insert(const float3 &position, float body_mass, uint32_t level)
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
            mass = body_mass;
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
                    children[i]->Insert(position, body_mass, level + 1);
                    break;
                }
            }

            float totalMass = body_mass + mass;
            center *= mass;
            center += body_mass * position;
            center *= (1.0f / totalMass);
            mass = totalMass; 
        }
    }
    else
    {
        float totalMass = body_mass + mass;
        center *= mass;
        center += body_mass * position;
        center *= (1.0f / totalMass);
        mass = totalMass;

        for (int i = 0; i < 4; i++)
        {
            if (children[i]->Contains(position))
            {
                children[i]->Insert(position, body_mass, level + 1);
                break;
            }
        }
    }
}

void BarnesHutGPUTree::InsertFlat(const float3& position, float body_mass)
{
    BarnesHutGPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    float3 insertedPosition[2] = { position };
    float insertedMass[2] = { body_mass };
    int8_t current = 0;
    
    while (count)
    {
        BarnesHutGPUTree& node = *stack[--count];

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

                float totalMass = body_mass + node.mass;
                node.center *= node.mass;
                node.center += body_mass * position;
                node.center *= (1.0f / totalMass);
                node.mass = totalMass;
            }
        }
        else
        {
            float totalMass = body_mass + node.mass;
            node.center *= node.mass;
            node.center += body_mass * position;
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

inline bool BarnesHutGPUTree::Contains(const float3 &position) const
{
    if (position.x >= point.x && position.x <= oppositePoint.x &&
        position.y >= point.y && position.y <= oppositePoint.y)
        return true;

    return false;
}

inline void BarnesHutGPUTree::ResetChildren()
{
    if (!children[0])
    {
        float nl = 0.5f * length;

        float x = point.x + nl;
        float y = point.y + nl;

        float3 np1(x, point.y, 0.0f);
        float3 np2(x, y, 0.0f);
        float3 np3(point.x, y, 0.0f);

        children[0] = make_unique<BarnesHutGPUTree>(point, nl);
        children[1] = make_unique<BarnesHutGPUTree>(np1, nl);
        children[2] = make_unique<BarnesHutGPUTree>(np2, nl);
        children[3] = make_unique<BarnesHutGPUTree>(np3, nl);
    }
    else
    {
        children[0]->Reset();
        children[1]->Reset();
        children[2]->Reset();
        children[3]->Reset();
    }
}

size_t BarnesHutGPUTree::GetNodesCount() const
{
    size_t nodesCount = 0;

    const BarnesHutGPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutGPUTree& node = *stack[--count];

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

float3 BarnesHutGPUTree::ComputeAcceleration(const float3& position, float soft) const
{
    float3 acceleration = {};

    if (isLeaf)
    {
        if (isBusy && !equal_eps(position, center, EPS))
        {
            acceleration = GravityAcceleration(center - position, mass, soft);
        }
    }
    else
    {
        float3 vec = center - position;
        float r = Math::length(vec);
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

float3 BarnesHutGPUTree::ComputeAccelerationFlat(const float3& position, float soft) const
{
    float3 acceleration = {};

    const BarnesHutGPUTree* stack[cNodesStackSize];
    size_t count = 1;

    stack[0] = this;

    while (count)
    {
        const BarnesHutGPUTree& node = *stack[--count];

        if (node.isLeaf)
        {
            if (node.isBusy && !equal_eps(position, node.center, EPS))
            {
                acceleration += GravityAcceleration(node.center - position, node.mass, soft);
            }
        }
        else
        {
            float3 vec = node.center - position;
            float r = Math::length(vec);
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
