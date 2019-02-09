#pragma once

#include "lpVec3.h"


// Сферически симметричное распределение материи

class SphericalModel
{
public:

	// Кол-во расчетных точек
	int		  m_N;
	// Расчетная область
	float	  m_gridXMin;
	float	  m_gridXMax;
	// Шаг разбиения
	float	  m_h;

	float     m_radius;

	// Гравитационный потенциал
	float	 *m_potential;
	// Напряженности гравитационного поля
	float	 *m_gravityField;

	float     m_potentialMax;
	float	  m_potentialMin;

	float	  m_vc;


	SphericalModel(float gridXMin, float gridXMax, float radius);
	~SphericalModel(void);

	// Расчет гравитационного потенциала
	void calcPotential();
	// Расчет напряженности
	void calcGravityField();

	lpVec3 getGravityVector(lpVec3 pos);
	float  getCircularVelocity(float r);

	void plotPotential();


	// Функция распределения плотности
	static float densityDistribution(float r);
	// Правая часть уравнения Пуассона
	static float rightPartPoisson(float r);

};

