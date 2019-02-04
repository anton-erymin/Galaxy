#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "GalaxyParticle.h"
#include "lpVec3.h"
#include "BarnesHutTree.h"
#include "SphericalModel.h"




#define		RAND					((float)rand() / RAND_MAX)
#define		RAND_MINMAX(a, b)		(a + (b - a) * RAND)




class GalaxySystem
{
public:
	// Массив частиц
	GalaxyParticle **m_particles;

	// Количество частиц
	int				 m_numAll;

	// Количество частиц в центальном балдже
	int				 m_numBulgeStars;

	float			 m_bulgeMass;
	// Радиус балджа
	float			 m_bulgeRadius;

	// Количество звезд в диске
	int				 m_numDiskStars;
	// Радиус диска
	float			 m_diskRadius;
	// Толщина диска
	float			 m_diskThickness;

	// Радиус гало темной материи
	float			 m_haloRadius;
	// Масса гало
	float			 m_haloMass;

	// Масса частицы
	float			 m_starMass;

	bool			 m_ccw;


	// Координаты центра
	lpVec3			 m_center;

	// Модель гало
	SphericalModel   m_darkMatter;




	// Конструктор
	GalaxySystem(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness, 
		float bulgeMass, float haloMass, float starMass, bool ccw);
	GalaxySystem();
	~GalaxySystem();


	// Создание галактики и ее инициализация
	void init();
	void createBulge();
	void createDisk();
	void setVelocities();

	// Брутфорсный алгоритм
	void step(float dt);

	// Алгоритм Барнса-Хата
	void stepBarnesHut(float dt);

	// Алгоритм Барнса-Хата с SSE оптимизацией
	void stepBarnesHutSIMD(float dt, BarnesHutTree *bht, GalaxySystem **galaxies, int numGalaxies);

	// Построение кваддерева
	void addToTree(BarnesHutTree *bht);

	// Создание частиц
	// Звездная частица
	void makeStar(GalaxyParticle *particle);
	// Пыль
	void makeDust(GalaxyParticle *particle);
	// Водород
	void makeH2(GalaxyParticle *particle);


	// Интегрирование частиц
	void stepParticles(float dt, GalaxyParticle **list, int num);

	// Обновление параметров частиц
	void update(float dt);



private:
	// Случайное число с нормальным распределением
	float randNormal();

};