#include "BarnesHutTree.h"

#include "Galaxy.h"

static constexpr uint32_t cMaxTreeLevel = 50;

BarnesHutTree::BarnesHutTree(const lpVec3 &point, float length)
    : point(point),
    length(length),
    isLeaf(true),
    particle_(nullptr),
    totalMass(0.0f)
{
    oppositePoint = point + lpVec3{ length };
}

void BarnesHutTree::Reset()
{
    isLeaf = true;
    particle_ = nullptr;
}

void BarnesHutTree::Insert(const Particle &p, uint32_t level)
{
    if (!Contains(p))
    {
        return;
    }

    // Вставка частицы в дерево
    if (level > cMaxTreeLevel)
    {
        return;
    }

    if (isLeaf)
    {
        // Если узел - лист
        if (!particle_)
        {
            // И пустой, то вставляем в него частицу
            particle_ = &p;
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

                lpVec3 np1(x, point.m_y, 0.0f);
                lpVec3 np2(x, y, 0.0f);
                lpVec3 np3(point.m_x, y, 0.0f);

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
                if (children[i]->Contains(*particle_))
                {
                    children[i]->Insert(*particle_, level + 1);
                    break;
                }
            }

            // И новую частицу
            for (int i = 0; i < 4; i++)
            {
                if (children[i]->Contains(p))
                {
                    children[i]->Insert(p, level + 1);
                    break;
                }
            }

            // Суммарная масса узла
            totalMass = particle_->mass + p.mass;

            // Центр тяжести
            massCenter = p.position.scaleR(p.mass);
            massCenter.addScaled(particle_->position, particle_->mass);
            massCenter *= 1.0f / totalMass;
        }
    }
    else
    {
        // Если это внутренний узел

        // Обновляем суммарную массу добавлением к ней массы новой частицы
        float total = totalMass + p.mass;

        // Также обновляем центр масс
        massCenter *= totalMass;
        massCenter.addScaled(p.position, p.mass);
        massCenter *= 1.0f / total;
        totalMass = total;

        // Рекурсивно вставляем в нужный потомок частицу
        for (int i = 0; i < 4; i++)
        {
            if (children[i]->Contains(p))
            {
                children[i]->Insert(p, level + 1);
                break;
            }
        }
    }
}

bool BarnesHutTree::Contains(const Particle &p) const
{
    lpVec3 v = p.position;
    if (v.m_x >= point.m_x && v.m_x <= oppositePoint.m_x &&
        v.m_y >= point.m_y && v.m_y <= oppositePoint.m_y)
        return true;

    return false;
}

lpVec3 BarnesHutTree::CalculateForce(const Particle &particle) const
{
    // Расчет силы действующей на частицу

    lpVec3 totalForce = {};

    float softFactor2 = 1.0e-0f;

    if (isLeaf && particle_)
    {
        if (particle_ != &particle)
        {
            // Если это лист и в нем содержится частица отличная от текущей
            // Вычисляем силу действующую между частицами
            lpVec3 force = particle_->position;
            force -= particle.position;

            float r2 = force.normSq();
            r2 += softFactor2;
            r2 *= r2 * r2;
            r2 = sqrtf(r2);

            force *= particle_->mass * particle.mass / r2;
            totalForce += force;

            return totalForce;
        }
    }

    if (!isLeaf)
    {
        // Если это внутренний узел

        // Находим расстояние от частицы до центра масс этого узла
        lpVec3 force = massCenter;
        force -= particle.position;

        float r = force.norm();

        // Находим соотношение размера узла к расстоянию
        float theta = length / r;

        if (theta < 0.7f)
        {
            float r2 = r * r + softFactor2;
            r2 *= r2 * r2;
            r2 = sqrtf(r2);

            // Если частица удалена от узла на достаточное расстояние
            // Рассматриваем узел как одну частицу с известной суммарной массой и центром масс
            // И Вычисляем силу
            force *= totalMass * particle.mass / r2;
            totalForce += force;
        }
        else
        {
            // Если частица близко к узлу рекурсивно считаем силу с потомками
            for (int i = 0; i < 4; i++)
            {
                totalForce += children[i]->CalculateForce(particle);
            }
        }
    }

    return totalForce;
}
