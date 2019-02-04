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
	// ������ ������
	GalaxyParticle **m_particles;

	// ���������� ������
	int				 m_numAll;

	// ���������� ������ � ���������� ������
	int				 m_numBulgeStars;

	float			 m_bulgeMass;
	// ������ ������
	float			 m_bulgeRadius;

	// ���������� ����� � �����
	int				 m_numDiskStars;
	// ������ �����
	float			 m_diskRadius;
	// ������� �����
	float			 m_diskThickness;

	// ������ ���� ������ �������
	float			 m_haloRadius;
	// ����� ����
	float			 m_haloMass;

	// ����� �������
	float			 m_starMass;

	bool			 m_ccw;


	// ���������� ������
	lpVec3			 m_center;

	// ������ ����
	SphericalModel   m_darkMatter;




	// �����������
	GalaxySystem(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness, 
		float bulgeMass, float haloMass, float starMass, bool ccw);
	GalaxySystem();
	~GalaxySystem();


	// �������� ��������� � �� �������������
	void init();
	void createBulge();
	void createDisk();
	void setVelocities();

	// ����������� ��������
	void step(float dt);

	// �������� ������-����
	void stepBarnesHut(float dt);

	// �������� ������-���� � SSE ������������
	void stepBarnesHutSIMD(float dt, BarnesHutTree *bht, GalaxySystem **galaxies, int numGalaxies);

	// ���������� ����������
	void addToTree(BarnesHutTree *bht);

	// �������� ������
	// �������� �������
	void makeStar(GalaxyParticle *particle);
	// ����
	void makeDust(GalaxyParticle *particle);
	// �������
	void makeH2(GalaxyParticle *particle);


	// �������������� ������
	void stepParticles(float dt, GalaxyParticle **list, int num);

	// ���������� ���������� ������
	void update(float dt);



private:
	// ��������� ����� � ���������� ��������������
	float randNormal();

};