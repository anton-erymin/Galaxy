#include "Universe.h"

#include <Windows.h>

#include "constants.h"
#include "texture.h"



TextureImage starTexture;
TextureImage dustTexture[3];


Universe::Universe(float size)
	: m_numGalaxies(0)
{
	m_galaxies = 0;
	m_bht	   = new BarnesHutTree(lpVec3(-size * 0.5f), size); 

	srand(GetTickCount());
}


Universe::~Universe(void)
{
	for (int i = 0; i < m_numGalaxies; i++)
		delete m_galaxies[i];
}


void Universe::init()
{
	for (int i = 0; i < m_numGalaxies; i++)
		m_galaxies[i]->init();
}


void Universe::addGalaxy(GalaxySystem *glx)
{
	if (m_galaxies == 0)
	{
		m_numGalaxies = 1;
		m_galaxies = new GalaxySystem*[m_numGalaxies];
		m_galaxies[0] = glx;
	}
	else
	{
		m_numGalaxies++;
		GalaxySystem **new_galaxies = new GalaxySystem*[m_numGalaxies];
		memcpy(new_galaxies, m_galaxies, (m_numGalaxies - 1) * sizeof(GalaxySystem*));
		new_galaxies[m_numGalaxies - 1] = glx;
		delete[] m_galaxies;
		m_galaxies = new_galaxies;
	}
}


void Universe::step(float dt)
{
	m_bht->m_leaf = true;
	m_bht->m_particle = 0;

	for (int i = 0; i < m_numGalaxies; i++) m_galaxies[i]->addToTree(m_bht);
	for (int i = 0; i < m_numGalaxies; i++) m_galaxies[i]->stepBarnesHutSIMD(dt, m_bht, m_galaxies, m_numGalaxies);
}


//void Universe::drawTree(BarnesHutTree *node)
//{
//	lpVec3 p = node->m_point;
//	float l = node->m_length;
//
//	glDisable(GL_BLEND);
//	glDisable(GL_TEXTURE_2D);
//
//	glBegin(GL_LINE_STRIP);
//		glColor3f(0.0f, 1.0f, 0.0f);
//
//		glVertex3f(p.m_x, p.m_y, 0.0f);
//		glVertex3f(p.m_x + l, p.m_y, 0.0f);
//		glVertex3f(p.m_x + l, p.m_y + l, 0.0f);
//		glVertex3f(p.m_x, p.m_y + l, 0.0f);
//		glVertex3f(p.m_x, p.m_y, 0.0f);
//	glEnd();
//
//	if (!node->m_leaf)
//	{
//		for (int i = 0; i < 4; i++)
//		{
//			drawTree(node->m_subs[i]);
//		}
//	}
//}