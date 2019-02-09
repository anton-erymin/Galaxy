#include "SphericalModel.h"

#include "Math.h"
#include "lpVec3.h"

#include <stdio.h>
#include <gl\glut.h>


float coeff;

float f(float x)
{
	return sin(x);
}

SphericalModel::SphericalModel(float gridXMin, float gridXMax, float radius)
	: m_gridXMin(gridXMin),
	  m_gridXMax(gridXMax),
	  m_radius(radius)
{
    calcPotential();
    calcGravityField();
}

void SphericalModel::calcPotential()
{
	float x = m_gridXMin;
	m_N = 10000;
	m_h = (m_gridXMax - m_gridXMin) / (m_N - 1);

	m_potential = new float[m_N];
	for (int i = 0; i < m_N; i++)
	{
		m_potential[i] = 0.0f;
	}

	// Решаем уравнение Пуассона для нахождения потенциала
	//bool res = poisson1d(5000, m_gridXMin, m_gridXMax, m_N, m_potential, rightPartPoisson);


	// Isothermal
	float M = 100.0f;
	float r0 = 20.0;
	float rho0 = M / (4.0f * M_PI * r0 * r0 * r0 * m_radius);
	m_vc = sqrtf(4.0f * M_PI * r0 * r0 * r0 * rho0);

	for (int i = 1; i < m_N; i++)
	{
		float r = m_gridXMin + i * m_h;
		m_potential[i] = 4.0f * M_PI * rho0 * r0 * r0 * r0 * log(r / r0);
	}

	m_potential[0] = 4 * m_potential[1];




	// Uniform
	/*float M = 100.0f;
	float a = m_radius;
	float rho0 = 3.0f * M / (4.0f * M_PI * powf(a, 3.0f));
	coeff = sqrtf(4.0f * M_PI * rho0 / 3.0f);

	for (int i = 0; i < m_N; i++)
	{
		float r = m_gridXMin + i * m_h;

		if (r < a)
		{
			m_potential[i] = -2.0f * M_PI * rho0 * (a * a - r * r / 3.0f);
		}
		else
		{
			m_potential[i] = -M / r;
		}

		
	}*/



	// Plummer
	/*float M = 100.0f;
	float a = 1.0;
	float asq = a * a;

	for (int i = 0; i < m_N; i++)
	{
		float r = m_gridXMin + i * m_h;

		m_potential[i] = - M / sqrtf(r * r + asq);
	}*/


}

void SphericalModel::calcGravityField()
{
	// Находим градиент потенциала
	m_gravityField = new float[m_N];

	float hd = 2.0f * m_h;

	m_potentialMin = 1.0e+38f;
	m_potentialMax = 0.0f;

	for (int i = 1; i < m_N - 1; i++)
	{
		m_gravityField[i] = -(m_potential[i + 1] - m_potential[i - 1]) / hd;

		float pot = m_potential[i];
		if (pot > m_potentialMax)
			m_potentialMax = pot;
		else if (pot < m_potentialMin)
			m_potentialMin = pot;
	}

	m_gravityField[0      ] = -(m_potential[1      ] - m_potential[0      ]) / m_h;
	m_gravityField[m_N - 1] = -(m_potential[m_N - 1] - m_potential[m_N - 2]) / m_h;
}

lpVec3 SphericalModel::getGravityVector(lpVec3 pos)
{
	float r = pos.normalize();

	int ix = (int)((r - m_gridXMin) / m_h);

	if (ix == 0)
	{
		int a = 1;
	}

	if (ix >= 0 && ix < m_N)
	{
		float g1 = m_gravityField[ix    ];
		float g2 = m_gravityField[ix + 1];
		float t = (r - (m_gridXMin + ix * m_h)) / m_h;
		pos *= g1 + t * (g2 - g1);
	}
	else pos.clear();
	return pos;
}

float SphericalModel::getCircularVelocity(float r)
{
	//float r2 = r * r;
	//return sqrtf(100.0f * (r2 - 1.0f) / sqrtf(r2 * r2 * r2));

	//return coeff * r;

	return m_vc;
}

float SphericalModel::densityDistribution(float r)
{
	//if (r > 30.0f) return 0.0f;

	//return 100.0f * exp(-r * 1e-3f);


	//float rho = 3.0f * 1e+12/(4.0f * M_PI * 100000.0f * 100000.0f * 100000.0f);
	//return 0.00024f;// * exp(-r * 0.000005);	

	return 10000.0f;
	return 3.0f * 100.0f / (4.0f * M_PI * 30.0f * 30.0f * 30.0f);
	float a = 16.0f;
	return 3.0f * 1000.0f/(4.0f * M_PI * a * a * a) / sqrtf( pow(1.0f + pow(r / a, 2), 5) );

}

float SphericalModel::rightPartPoisson(float r)
{
	return 4.0f * M_PI * densityDistribution(r);
}

void SphericalModel::plotPotential()
{
	float x = m_gridXMin;

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINE_STRIP);

	float scale = 10.0f / m_potentialMin;
	for (int i = 0; i < m_N; i++)
	{
		
		float p = m_potential[i];
		float ps = scale * p;
		float dpi = 1.0f / (m_potentialMax - m_potentialMin);
		glColor3f((0.5f*p - m_potentialMin) * dpi, 1.0f - (p - m_potentialMin) * dpi, 0.0f);
		glVertex3f(x + i * m_h, -ps, 0.0f);
	}
	glEnd();
}

SphericalModel::~SphericalModel(void)
{
	delete[] m_gravityField;
	delete[] m_potential;
}