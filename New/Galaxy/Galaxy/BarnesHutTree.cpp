#include "BarnesHutTree.h"
#include "GalaxySystem.h"
#include "constants.h"


int curDepth;


BarnesHutTree::BarnesHutTree(const lpVec3 &point, float length)
	: m_point(point),
	  m_length(length),
	  m_leaf(true),
	  m_particle(0),
	  m_subs(0),
	  m_totalMass(0.0f)
{
	m_oppositePoint = m_point + lpVec3(m_length);
}


void BarnesHutTree::insert(GalaxyParticle *p)
{
	// Вставка частицы в дерево
	if (curDepth > 40)
	{
		return;
	}

	if (m_leaf)
	{
		// Если узел - лист
		if (!m_particle)
		{
			// И пустой, то вставляем в него частицу
			m_particle = p;
			return;
		}
		else
		{
			// Если лист непустой он становится внутренним узлом			
			m_leaf = false;

			if (!m_subs)
			{
				// Если потомки еще не были созданы создаем их
				m_subs = new BarnesHutTree*[4];

				// Размеры потомков в половину меньше
				float nl = 0.5f * m_length;

				float x = m_point.m_x + nl;
				float y = m_point.m_y + nl;

				lpVec3 np1(x, m_point.m_y, 0.0f);
				lpVec3 np2(x, y, 0.0f);
				lpVec3 np3(m_point.m_x, y, 0.0f);
			
				m_subs[0] = new BarnesHutTree(m_point,  nl);
				m_subs[1] = new BarnesHutTree(np1,		nl);
				m_subs[2] = new BarnesHutTree(np2,		nl);
				m_subs[3] = new BarnesHutTree(np3,		nl);
			}
			else
			{
				// Иначе сбрасываем их
				m_subs[0]->m_leaf = true;
				m_subs[1]->m_leaf = true;
				m_subs[2]->m_leaf = true;
				m_subs[3]->m_leaf = true;

				m_subs[0]->m_particle = 0;
				m_subs[1]->m_particle = 0;
				m_subs[2]->m_particle = 0;
				m_subs[3]->m_particle = 0;
			}
			
			// Далее вставляем в нужный потомок частицу которая была в текущем узле
			for (int i = 0; i < 4; i++)
			{
				if (m_subs[i]->contains(m_particle))
				{
					m_subs[i]->insert(m_particle);
					break;
				}
			}

			// И новую частицу
			for (int i = 0; i < 4; i++)
			{
				if (m_subs[i]->contains(p))
				{
					curDepth++;
					m_subs[i]->insert(p);
					break;
				}
			}

			// Суммарная масса узла
			m_totalMass = m_particle->m_mass + p->m_mass;

			// Центр тяжести
			m_mc = p->m_pos.scaleR(p->m_mass);
			m_mc.addScaled(m_particle->m_pos, m_particle->m_mass);
			m_mc *= 1.0f / m_totalMass;		
		}
	}
	else
	{
		// Если это внутренний узел

		// Обновляем суммарную массу добавлением к ней массы новой частицы
		float total = m_totalMass + p->m_mass;

		// Также обновляем центр масс
		m_mc *= m_totalMass;
		m_mc.addScaled(p->m_pos, p->m_mass);
		m_mc *= 1.0f / total;		
		m_totalMass = total;

		// Рекурсивно вставляем в нужный потомок частицу
		for (int i = 0; i < 4; i++)
		{
			if (m_subs[i]->contains(p))
			{
				curDepth++;
				m_subs[i]->insert(p);
				break;
			}
		}
	}
}


bool BarnesHutTree::contains(GalaxyParticle *p)
{
	lpVec3 v = p->m_pos;
	if (v.m_x >= m_point.m_x && v.m_x <= m_oppositePoint.m_x &&
		v.m_y >= m_point.m_y && v.m_y <= m_oppositePoint.m_y)
		return true;

	return false;
}


void BarnesHutTree::calcForce(GalaxyParticle *p)
{
	// Расчет силы действующей на частицу

	float softFactor2 = 1.0e-0f;

	if (m_leaf && m_particle)
	{
		if (m_particle != p)
		{
			// Если это лист и в нем содержится частица отличная от текущей
			// Вычисляем силу действующую между частицами
			lpVec3 force = m_particle->m_pos;
			force -= p->m_pos;

			float r2 = force.normSq();
			r2 += softFactor2;
			r2 *= r2 * r2;
			r2 = sqrtf(r2);

			force *= m_particle->m_mass * p->m_mass / r2;
			p->m_forceAccum += force;

			return;
		}
	}


	if (!m_leaf)
	{
		// Если это внутренний узел

		// Находим расстояние от частицы до центра масс этого узла
		lpVec3 force = m_mc;
		force -= p->m_pos;

		float r = force.norm();

		// Находим соотношение размера узла к расстоянию
		float theta = m_length / r;

		if (theta < 0.7f)
		{
			float r2 = r * r + softFactor2;
			r2 *= r2 * r2;
			r2 = sqrtf(r2);

			// Если частица удалена от узла на достаточное расстояние
			// Рассматриваем узел как одну частицу с известной суммарной массой и центром масс
			// И Вычисляем силу
			force *= m_totalMass * p->m_mass / r2;
			p->m_forceAccum += force;
		}
		else
		{
			// Если частица близко к узлу рекурсивно считаем силу с потомками
			for (int i = 0; i < 4; i++)
			{
				m_subs[i]->calcForce(p);
			}
		
		}

	}

}