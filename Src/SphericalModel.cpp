#include "SphericalModel.h"

#include "Math.h"
#include "lpVec3.h"

#include <cassert>
#include <algorithm>

#include <gl\glut.h>


static constexpr uint32_t N = 10000;

float f(float x)
{
	return sin(x);
}

SphericalModel::SphericalModel(float gridXMin, float gridXMax, float radius)
	: rmin(gridXMin),
	  rmax(gridXMax),
	  radius(radius)
{
    r.resize(N);
    rho.resize(N, 0.0f);
    rightPartPoisson.resize(N);
    potential.resize(N, 0.0f);
    field.resize(N, 0.0f);

    h = (rmax - rmin) / (N - 1);
    for (size_t i = 0; i < r.size(); ++i)
    {
        r[i] = rmin + i * h;
    }

    CalculatePotential();
    CalculateGravityField();
}

void SphericalModel::CalculatePotential()
{
    float rho0 = 10.0;

    for (size_t i = 0; i < r.size(); ++i)
    {
        rho[i] = rho0 / (1.0f + (r[i] / radius) * (r[i] / radius));
        rightPartPoisson[i] = 4.0f * M_PI * rho[i];
    }


	// Решаем уравнение Пуассона для нахождения потенциала
    Poisson1(100, rmin, rmax, N, potential.data(), rightPartPoisson);

	// Isothermal
	/*float M = 100.0f;
	float r0 = 20.0;
	float rho0 = M / (4.0f * M_PI * r0 * r0 * r0 * radius);
	vc = sqrtf(4.0f * M_PI * r0 * r0 * r0 * rho0);

	for (int i = 0; i < N; i++)
	{
		float r = rmin + i * h;
		potential[i] = 4.0f * M_PI * rho0 * r0 * r0 * r0 * log(r / r0);
	}*/

	//potential[0] = 4 * potential[1];

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

void SphericalModel::CalculateGravityField()
{
	// Находим градиент потенциала

	float hd = 2.0f * h;

	for (int i = 1; i < N - 1; i++)
	{
		field[i] = -(potential[i + 1] - potential[i - 1]) / hd;

		float pot = potential[i];
	}

	field[0      ] = -(potential[1      ] - potential[0      ]) / h;
	field[N - 1] = -(potential[N - 1] - potential[N - 2]) / h;
}

lpVec3 SphericalModel::GetForce(lpVec3 pos) const
{
	float r = pos.normalize();

	int ix = (int)((r - rmin) / h);

	if (ix == 0)
	{
		int a = 1;
	}

	if (ix >= 0 && ix < N)
	{
		float g1 = field[ix    ];
		float g2 = field[ix + 1];
		float t = (r - (rmin + ix * h)) / h;
		pos *= g1 + t * (g2 - g1);
	}
	else pos.clear();
	return pos;
}

float SphericalModel::GetCircularVelocity(float r) const
{
	//float r2 = r * r;
	//return sqrtf(100.0f * (r2 - 1.0f) / sqrtf(r2 * r2 * r2));

	//return coeff * r;

	return vc;
}

static float DensityDistribution(float r)
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

static float RightPartPoisson(float r)
{
	return 4.0f * M_PI * DensityDistribution(r);
}

static void Plot(const std::vector<float> x, const std::vector<float> y, float xscale, float yscale)
{
    assert(x.size() == y.size());

    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::min();

    for (const auto& value : y)
    {
        minValue = std::min(value, minValue);
        maxValue = std::max(value, maxValue);
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (size_t i = 0; i < x.size(); ++i)
    {
        float color = (y[i] - minValue) / (maxValue - minValue);
        color = 0.2f * (1.0f - color) + color;
        glColor3f(color, color, color);
        glVertex3f(x[i] * xscale, y[i] * yscale, 0.0f);
    }

    glEnd();
}

void SphericalModel::PlotPotential() const
{
    Plot(r, potential, 1.0f, 300.0f);
    Plot(r, field, 1.0f, 300.0f);
    //Plot(r, potential, 1.0f, 1.0f);

	/*float x = rmin;

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINE_STRIP);

	float scale = 10.0f / potentialMin;
	for (int i = 0; i < N; i++)
	{
		float p = potential[i];
		float ps = scale * p;
		float dpi = 1.0f / (potentialMax - potentialMin);
		glColor3f((0.5f*p - potentialMin) * dpi, 1.0f - (p - potentialMin) * dpi, 0.0f);
		glVertex3f(x + i * h, -ps, 0.0f);
	}
	glEnd();*/
}
