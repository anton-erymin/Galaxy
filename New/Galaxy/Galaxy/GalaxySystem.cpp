#include "GalaxySystem.h"

#include <string.h>
#include <gl\glut.h>

#include "constants.h"
#include "poisson.h"
#include "texture.h"


extern int curDepth;

extern TextureImage starTexture;
extern TextureImage dustTexture[3];


float **rhoData;
float maxp, minp;

float size;

int curLayer = 0;



GalaxySystem::GalaxySystem() : m_darkMatter(0.0f, 0.0f, 0.0f)
{
	m_center = lpVec3();

	m_bulgeRadius = GLX_BULGE_RADIUS;
	m_numBulgeStars = GLX_BULGE_NUM;
	m_bulgeMass = GLX_BULGE_MASS;
	m_diskRadius = GLX_DISK_RADIUS;
	m_numDiskStars = GLX_DISK_NUM;
	m_haloRadius = GLX_HALO_RADIUS;
	m_haloMass = GLX_HALO_MASS;
	m_diskThickness = GLX_DISK_THICKNESS;
	m_starMass = GLX_STAR_MASS;
	
	m_numAll = 0;
	m_ccw = true;
}


GalaxySystem::GalaxySystem(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness, 
		float bulgeMass, float haloMass, float starMass, bool ccw)
		: m_center(center),
		  m_numBulgeStars(numBulgeStars),
		  m_numDiskStars(numDiskStars),
		  m_bulgeRadius(bulgeRadius),
		  m_diskRadius(diskRadius),
		  m_haloRadius(haloRadius),
		  m_diskThickness(diskThickness),
		  m_bulgeMass(bulgeMass),
		  m_haloMass(haloMass),
		  m_starMass(starMass),
		  m_ccw(ccw),
		  m_darkMatter(0.0f, 10.0f * haloRadius, haloRadius)
{
	
}


void GalaxySystem::init()
{
	m_numAll = m_numBulgeStars + m_numDiskStars;

	m_particles	= new GalaxyParticle*[m_numAll];


	// Инициализируем массивы частиц
	for (int i = 0; i < m_numAll; i++)
		m_particles[i] = 0;


	m_darkMatter.init();

	createBulge();
	createDisk();
}


void GalaxySystem::createBulge()
{
	lpVec3 pos, vel;
	float r, phi, theta, speed;
	GalaxyParticle *particle;


	int numDusts = m_numBulgeStars / 4;

	for (int i = 0; i < m_numBulgeStars; i++)
	{
		particle = new GalaxyParticle(m_starMass);
		m_particles[i] = particle;

		particle->m_texID = starTexture.texID;

		if (i < numDusts)
			makeDust(particle);
		else makeStar(particle);

		// Случайные сферические координаты
		r       = m_bulgeRadius * RAND;
		phi     = 2.0f * M_PI * RAND;
		theta   = 2.0f * M_PI * RAND;

		pos.m_x		  = r * sinf(theta) * cosf(phi);
		pos.m_y       = r * sinf(theta) * sinf(phi);
		pos.m_z		  = r * cosf(theta);

		pos += m_center;
		particle->m_pos = pos;

		//speed = m_darkMatter.getCircularVelocity(r);
		//vel.setTo(pos.m_y, -pos.m_x, 0.0f);
		//vel.setTo(RAND_MINMAX(-1.0f, 1.0f), RAND_MINMAX(-1.0f, 1.0f), RAND_MINMAX(-1.0f, 1.0f));
		//vel.normalize();
		//vel *= speed;

		//particle->m_linearVel = vel;
	}

	m_particles[0]->m_pos = m_center;
}


void GalaxySystem::createDisk()
{
	lpVec3 pos, vel;
	float r, phi, speed, d;

	int numDusts = m_numDiskStars / 4;

	for (int i = 0; i < m_numDiskStars; i++)
	{
		GalaxyParticle *particle = new GalaxyParticle(m_starMass);
		m_particles[m_numBulgeStars + i] = particle;

		if (i < numDusts)

			makeDust(particle);
		else makeStar(particle);

		// Случайные цилиндрические координаты
		r       = RAND_MINMAX(0.99f * m_bulgeRadius, m_diskRadius);
		//r = 0.99f * m_bulgeRadius + (m_diskRadius - 0.99f * m_bulgeRadius) * randNormal();
		phi     = 2.0f * M_PI * RAND;

		pos.m_x		  = r * cosf(phi);
		pos.m_y       = r * sinf(phi);
		pos.m_z		  = (2.0f * RAND - 1.0f) * m_diskThickness;

		pos += m_center;
		particle->m_pos = pos;


		//speed = m_darkMatter.getCircularVelocity(r);
		//vel.setTo(pos.m_y, -pos.m_x, 0.0f);
		//vel.normalize();
		//vel *= speed;
		//d = 0.1 * speed;
		//vel += lpVec3(d * RAND_MINMAX(-1.0f, 1.0f), d * RAND_MINMAX(-1.0f, 1.0f), d * RAND_MINMAX(-1.0f, 1.0f));

		//particle->m_linearVel = vel;
	}
}


void GalaxySystem::makeStar(GalaxyParticle *particle)
{
	particle->m_size = RAND_MINMAX(0.1f, 0.4f);
	particle->m_mag  = RAND_MINMAX(0.2f, 0.3f); 
	int k = rand() % 3;
	float rnd = RAND;

	switch (k)
	{
	case 0:
		particle->m_cR = 1.0f;
		particle->m_cG = 1.0f - rnd * 0.2f;
		particle->m_cB = 1.0f - rnd * 0.2f;
		break;
	case 1:
		particle->m_cR = 1.0f;
		particle->m_cG = 1.0f;
		particle->m_cB = 1.0f - rnd * 0.2f;
		break;
	case 2:
		particle->m_cR = 1.0f - rnd * 0.2f;
		particle->m_cG = 1.0f - rnd * 0.2f;
		particle->m_cB = 1.0f;
		break;
	}

	particle->m_texID = starTexture.texID;
}


void GalaxySystem::makeDust(GalaxyParticle *particle)
{
	particle->m_size = RAND_MINMAX(4.0f, 7.5f);
	particle->m_mag  = RAND_MINMAX(0.015f, 0.02f);
		//particle->m_size = 15;
		//particle->m_mag = 1;
	int k = rand() % 3;
	k = 1;
	if (k == 0)
	{
		particle->m_cR = 0.77f;
		particle->m_cG = 0.8f;
	}
	else
	{
		particle->m_cB = 0.8f;
		particle->m_cG = 0.95f;
	}

	particle->m_texID = dustTexture[1].texID;
}


void GalaxySystem::makeH2(GalaxyParticle *particle)
{
	particle->m_size = RAND_MINMAX(0.2f, 0.6f);
	particle->m_mag  = RAND_MINMAX(0.0f, 1.0f);
		
	particle->m_cR = 1.0f;
	particle->m_cG = 0.6f;
	particle->m_cB = 0.6f;

	particle->m_userData = rand() % 2;
}


void GalaxySystem::step(float dt)
{
	// Bruteforce

	/*GalaxyParticle *p1, *p2;

	for (int i = 0; i < GLX_NUM_PARTICLES; i++)
	{
		p1 = m_particles[i];

		for (int j = i + 1; j < GLX_NUM_PARTICLES; j++)
		{
			p2 = m_particles[j];
			lpVec3 force = p2->m_pos;
			force -= p1->m_pos;
			float r = force.normSq();

			if (r < 50000.0f)
			{
				r += 50000.0f;
			}

			force *= (1.0f / sqrtf(r)) * (G_CONST * p1->m_mass * p2->m_mass / r);

			p1->m_forceAccum += force;
			p2->m_forceAccum -= force;
		}
	}



	for (int i = 1; i < GLX_NUM_PARTICLES; i++)
	{
		m_particles[i]->integrate(dt);
	}*/
}



void GalaxySystem::addToTree(BarnesHutTree *bht)
{
	/*for (int i = 0; i < m_numAll; i++)
	{
		curDepth = 0;
		bht->insert(m_particles[i]);
	}*/

	//curDepth = 0;
	//bht->insert(m_particles[0]);
}



void GalaxySystem::stepBarnesHut(float dt)
{
	/*DWORD t = GetTickCount();

	buildTree();

	for (int i = 1; i < GLX_NUM_PARTICLES; i++)
	{
		m_bht->calcForce(m_particles[i]);
	}

	for (int i = 1; i < GLX_NUM_PARTICLES; i++)
	{
		m_particles[i]->integrate(dt);
	}
*/
	
	//printf("Time: %f sec\n", (float)(GetTickCount() - t) * 0.001f);
}



void GalaxySystem::stepBarnesHutSIMD(float dt, BarnesHutTree *bht, GalaxySystem **galaxies, int numGalaxies)
{
	// Считаем силы взаимодействия между звездами и ядром
	for (int i = 0; i < m_numAll; i++) bht->calcForce(m_particles[i]);

	
	// Считаем действие на звезды темного гало

	for (int i = 0; i < numGalaxies; i++)
	{
		lpVec3 center = galaxies[i]->m_center;

		for (int j = 0; j < m_numAll; j++)
		{
			lpVec3 pos = m_particles[j]->m_pos;
			pos -= center;
			//m_particles[j]->m_forceAccum.addScaled(galaxies[i]->m_darkMatter.getGravityVector(pos), m_particles[j]->m_mass);		
		}
	}


	stepParticles(dt, m_particles, m_numAll);
	update(dt);
}



void GalaxySystem::stepParticles(float dt, GalaxyParticle **list, int num)
{
	// Интегрирование

	GalaxyParticle *p;
	for (int i = 0; i < num; i++)
	{
		p = list[i];
		float *force = &p->m_forceAccum.m_x;
		float *linVel = &p->m_linearVel.m_x;
		float *pos = &p->m_pos.m_x;
		float imdt = p->m_invMass * dt;
		
		__asm
		{
			mov			eax, force
			movups		xmm0, [eax]		// xmm0 - force
			mov			ebx, linVel
			movups		xmm1, [ebx]		// xmm1 - velocity
			mov			ecx, pos
			movups		xmm2, [ecx]		// xmm2 - pos
			
			movss		xmm3, imdt
			movss		xmm4, dt
			shufps		xmm3, xmm3, 0	// xmm3 - imdt imdt imdt imdt
			shufps		xmm4, xmm4, 0	// xmm4 - dt dt dt dt

			mulps		xmm0, xmm3		// (dv) force *= imdt
			addps		xmm1, xmm0		// velocity += dv
			movups		xmm5, xmm1		// xmm5 - vel
			mulps		xmm1, xmm4		// (dp) velocity *= dt
			addps		xmm2, xmm1		// pos += dp

			xorps		xmm0, xmm0		// force = 0

			movups		[eax], xmm0
			movups		[ebx], xmm5
			movups		[ecx], xmm2
			

		}

	}
}



void GalaxySystem::update(float dt)
{
	//m_center = m_particles[0]->m_pos;


	/*float dmag = 0.01f;

	int off = m_numStars + m_numDusts;

	for (int i = 0; i < m_numH2; i++)
	{
		GalaxyParticle *p = m_particles[i + off];

		if (!p->m_active)
		{
			p->m_timer += dt;
			if (p->m_timer > p->m_activationTime)
			{
				p->m_active = true;
				p->m_userData = 1;
			}
			continue;
		}

		if (p->m_userData == 1)
		{
			p->m_mag += dmag;
			if (p->m_mag > 1.0f)
			{
				p->m_mag = 1.0f;
				p->m_userData = 0;
			}
		}
		else
		{
			p->m_mag -= dmag;
			if (p->m_mag < 0.0f)
			{
				p->m_mag = 0.0f;
				p->m_active = false;
				p->m_timer = 0.0f;
				p->m_activationTime = RAND_MINMAX(50.0f, 500.0f) * dt;
			}
		}
	}*/
}



float GalaxySystem::randNormal()
{
	float v1 = 2.0f * RAND - 1.0f;
	float v2 = 2.0f * RAND - 1.0f;
	float r = v1 * v1 + v2 * v2;

	while (r >= 1.0f || r < 0.0000001f)
	{
		v1 = 2.0f * RAND - 1.0f;
		v2 = 2.0f * RAND - 1.0f;
		r = v1 * v1 + v2 * v2;
	}

	float fac = sqrtf(-2.0f * logf(r) / r);
	return v1 * fac;
}



GalaxySystem::~GalaxySystem()
{
	for (int i = 0; i < m_numAll; i++)
	{
		delete m_particles[i];
	}
	delete[] m_particles;
}