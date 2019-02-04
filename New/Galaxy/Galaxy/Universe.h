#pragma once

#include "GalaxySystem.h"
#include "BarnesHutTree.h"


class Universe
{
public:

	// ���������
	GalaxySystem		**m_galaxies;
	// ���������� �������� � �������
	int					m_numGalaxies;

	// ����������
	BarnesHutTree		*m_bht;



	Universe(float size);
	~Universe(void);

	void init();
	void addGalaxy(GalaxySystem *glx);
	void step(float dt);
};

