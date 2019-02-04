#pragma once

#include "GalaxySystem.h"
#include "BarnesHutTree.h"


class Universe
{
public:

	// Галактики
	GalaxySystem		**m_galaxies;
	// Количество галактик в системе
	int					m_numGalaxies;

	// Октодерево
	BarnesHutTree		*m_bht;



	Universe(float size);
	~Universe(void);

	void init();
	void addGalaxy(GalaxySystem *glx);
	void step(float dt);
};

