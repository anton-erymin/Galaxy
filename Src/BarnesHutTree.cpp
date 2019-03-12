#include "BarnesHutTree.h"

#include "Math.h"

static constexpr uint32_t cMaxTreeLevel = 50;

BarnesHutTree::BarnesHutTree(const float3 &point, float length)
    : point(point),
    length(length)
{
    oppositePoint = point + float3{ length };
}

void BarnesHutTree::Reset()
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
        return;
    }

    if (isLeaf)
    {
        // Если узел - лист
        if (!isBusy)
        {
            // И пустой, то вставляем в него частицу
            center = position;
            this->mass = bodyMass;
            isBusy = true;
            return;
        }
        else
        {
            // Если лист непустой он становится внутренним узлом			
            isLeaf = false;

            if (!children[0])
            {
                // Размеры потомков в половину меньше
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
                // Иначе сбрасываем их
                children[0]->Reset();
                children[1]->Reset();
                children[2]->Reset();
                children[3]->Reset();
            }

            // Далее вставляем в нужный потомок частицу которая была в текущем узле
            for (int i = 0; i < 4; i++)
            {
                if (children[i]->Contains(center))
                {
                    children[i]->Insert(center, mass, level + 1);
                    break;
                }
            }

            // И новую частицу
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
        // Если это внутренний узел

        float totalMass = bodyMass + mass;
        center.scale(mass);
        center.addScaled(position, bodyMass);
        center *= (1.0f / totalMass);
        mass = totalMass;

        // Рекурсивно вставляем в нужный потомок частицу
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

inline bool BarnesHutTree::Contains(const float3 &position) const
{
    if (position.m_x >= point.m_x && position.m_x <= oppositePoint.m_x &&
        position.m_y >= point.m_y && position.m_y <= oppositePoint.m_y)
        return true;

    return false;
}

float3 BarnesHutTree::ComputeAcceleration(const float3& position, float softFactor) const
{
    float3 acceleration = {};

    if (isLeaf && isBusy)
    {
        if (!equal(position, center, 0.00001f))
        {
            acceleration = GravityAcceleration(center - position, mass, softFactor);
        }
    }
    else if (!isLeaf)
    {
        // Если это внутренний узел

        // Находим расстояние от частицы до центра масс этого узла
        float3 vec = center - position;
        float r = vec.norm();

        // Находим соотношение размера узла к расстоянию
        float theta = length / r;

        if (theta < 0.7f)
        {
            acceleration = GravityAcceleration(vec, mass, softFactor, r);
        }
        else
        {
            // Если частица близко к узлу рекурсивно считаем силу с потомками
            for (int i = 0; i < 4; i++)
            {
                acceleration += children[i]->ComputeAcceleration(position, softFactor);
            }
        }
    }

    return acceleration;
}
