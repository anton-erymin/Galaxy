#include "GalaxySystem.h"

#include <string.h>
#include "gl\glut.h"

#include "constants.h"
#include "poisson.h"
#include "texture.h"

extern int curDepth;
extern TextureImage starTexture;

float **rhoData;
float maxp, minp;

float size;

int curLayer = 0;

GalaxySystem::GalaxySystem() : darkMatter_(0.0f, 0.0f, 0.0f) {
    center_ = lpVec3();

    bulgeRadius_ = GLX_BULGE_RADIUS;
    numBulgeStars_ = GLX_BULGE_NUM;
    bulgeMass_ = GLX_BULGE_MASS;
    diskRadius_ = GLX_DISK_RADIUS;
    numDiskStars_ = GLX_DISK_NUM;
    haloRadius_ = GLX_HALO_RADIUS;
    haloMass_ = GLX_HALO_MASS;
    diskThickness_ = GLX_DISK_THICKNESS;
    starMass_ = GLX_STAR_MASS;

    ccw_ = true;
}

GalaxySystem::GalaxySystem(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness,
    float bulgeMass, float haloMass, float starMass, bool ccw)
    : center_(center),
    numBulgeStars_(numBulgeStars),
    numDiskStars_(numDiskStars),
    bulgeRadius_(bulgeRadius),
    diskRadius_(diskRadius),
    haloRadius_(haloRadius),
    diskThickness_(diskThickness),
    bulgeMass_(bulgeMass),
    haloMass_(haloMass),
    starMass_(starMass),
    ccw_(ccw),
    darkMatter_(0.0f, 10.0f * haloRadius, haloRadius) {
    
    init();
}

void GalaxySystem::init() {

    particles_.resize(numBulgeStars_ + numDiskStars_);

    darkMatter_.init();

    createBulge();
    createDisk();
}

void GalaxySystem::createBulge() {
    lpVec3 pos, vel;
    float r, phi, theta, speed;

    int numDusts = numBulgeStars_ / 4;

    for (int i = 0; i < numBulgeStars_; ++i) {
        auto &particle   = particles_[i];
        particle         = { starMass_ };
        particle.m_texID = starTexture.texID;

        if (i < numDusts) {
            makeDust(particle);
        }
        else {
            makeStar(particle);
        }

        // Случайные сферические координаты
        r = bulgeRadius_ * RAND;
        phi = 2.0f * M_PI * RAND;
        theta = 2.0f * M_PI * RAND;

        pos.m_x = r * sinf(theta) * cosf(phi);
        pos.m_y = r * sinf(theta) * sinf(phi);
        pos.m_z = r * cosf(theta);

        pos += center_;
        particle.m_pos = pos;

        speed = darkMatter_.getCircularVelocity(r);
        //vel.setTo(pos.m_y, -pos.m_x, 0.0f);
        vel.setTo(RAND_MINMAX(-1.0f, 1.0f), RAND_MINMAX(-1.0f, 1.0f), RAND_MINMAX(-1.0f, 1.0f));
        vel.normalize();
        vel *= speed;

        particle.m_linearVel = vel;
    }

    particles_[0].m_pos = center_;
}

void GalaxySystem::createDisk() {
    lpVec3 pos, vel;
    float r, phi, speed, d;

    int numDusts = numDiskStars_ / 2;

    for (int i = 0; i < numDiskStars_; i++) {
        auto &particle = particles_[numBulgeStars_ + i];
        particle       = { starMass_ };
    
        particle.m_texID = starTexture.texID;

        if (i < numDusts)
            makeDust(particle);
        else makeStar(particle);

        // Случайные цилиндрические координаты
        r = RAND_MINMAX(0.5f * bulgeRadius_, diskRadius_);
        phi = 2.0f * M_PI * RAND;

        pos.m_x = r * cosf(phi);
        pos.m_y = r * sinf(phi);
        pos.m_z = (2.0f * RAND - 1.0f) * diskThickness_;

        pos += center_;
        particle.m_pos = pos;


        speed = darkMatter_.getCircularVelocity(r);
        vel.setTo(pos.m_y, -pos.m_x, 0.0f);
        vel.normalize();
        vel *= speed;
        d = 0.1 * speed;
        vel += lpVec3(d * RAND_MINMAX(-1.0f, 1.0f), d * RAND_MINMAX(-1.0f, 1.0f), d * RAND_MINMAX(-1.0f, 1.0f));

        particle.m_linearVel = vel;
    }
}

void GalaxySystem::makeStar(GalaxyParticle &particle) {
    particle.m_size = RAND_MINMAX(0.04f, 0.16f);
    particle.m_mag = RAND_MINMAX(0.2f, 0.7f);

    int k = rand() % 3;
    switch (k) {
    case 0:
        particle.m_cR = 1.0f;
        particle.m_cG = 1.0f - RAND * 0.1f;
        particle.m_cB = 1.0f - RAND * 0.1f;
        break;
    case 1:
        particle.m_cR = 1.0f;
        particle.m_cG = 1.0f;
        particle.m_cB = 1.0f - RAND * 0.1f;
        break;
    case 2:
        particle.m_cR = 1.0f - RAND * 0.1f;
        particle.m_cG = 1.0f - RAND * 0.1f;
        particle.m_cB = 1.0f;
        break;
    }
}

void GalaxySystem::makeDust(GalaxyParticle &particle) {
    particle.m_size = RAND_MINMAX(3.0f, 5.5f);
    particle.m_mag = RAND_MINMAX(0.015f, 0.02f);

    int k = rand() % 3;
    k = 0;
    if (k == 0) {
        particle.m_cR = 0.77f;
        particle.m_cG = 0.8f;
    } else {
        particle.m_cB = 0.9f;
    }
}

void GalaxySystem::makeH2(GalaxyParticle &particle) {
    particle.m_size = RAND_MINMAX(0.2f, 0.6f);
    particle.m_mag = RAND_MINMAX(0.0f, 1.0f);

    particle.m_cR = 1.0f;
    particle.m_cG = 0.6f;
    particle.m_cB = 0.6f;

    particle.m_userData = rand() % 2;
}

void GalaxySystem::drawParticles(lpVec3 v1, lpVec3 v2) const {
    lpVec3 p1, p2, p3, p4;
    for (const auto &x : particles_)
    {
        if (!x.m_active) {
            continue;
        }

        lpVec3 pos = x.m_pos;
        float s = 0.5f * x.m_size;
        float mag = x.m_mag;

        p1 = pos - v1 * s - v2 * s;
        p2 = pos - v1 * s + v2 * s;
        p3 = pos + v1 * s + v2 * s;
        p4 = pos + v1 * s - v2 * s;

        glBegin(GL_QUADS);
        glColor3f(x.m_cR * mag, x.m_cG * mag, x.m_cB * mag);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(p1.m_x, p1.m_y, p1.m_z);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(p2.m_x, p2.m_y, p2.m_z);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(p3.m_x, p3.m_y, p3.m_z);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(p4.m_x, p4.m_y, p4.m_z);

        if (x.m_doubleDrawing) {
            glTexCoord2f(0.0f, 1.0f); glVertex3fv(&p1.m_x);
            glTexCoord2f(0.0f, 0.0f); glVertex3fv(&p2.m_x);
            glTexCoord2f(1.0f, 0.0f); glVertex3fv(&p3.m_x);
            glTexCoord2f(1.0f, 1.0f); glVertex3fv(&p4.m_x);
        }
        glEnd();

        glPopMatrix();
    }
}

void GalaxySystem::draw(int mode) const {
    // Отрисовка галактики

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDepthMask(false);
    glBindTexture(GL_TEXTURE_2D, starTexture.texID);

    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    lpVec3 v1 = lpVec3(modelview[0], modelview[4], modelview[8]);
    lpVec3 v2 = lpVec3(modelview[1], modelview[5], modelview[9]);

    drawParticles(v1, v2);

    glDepthMask(true);

    darkMatter_.plotPotential();
}

void GalaxySystem::step(float dt) {
    // Bruteforce

    /*GalaxyParticle *p1, *p2;

    for (int i = 0; i < GLX_NUM_PARTICLES; i++)
    {
        p1 = particles_[i];

        for (int j = i + 1; j < GLX_NUM_PARTICLES; j++)
        {
            p2 = particles_[j];
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
        particles_[i]->integrate(dt);
    }*/
}

void GalaxySystem::addToTree(BarnesHutTree &bht) const {
    for (auto &x : particles_) {
        curDepth = 0;
        bht.insert(x);
    }

    //curDepth = 0;
    //bht->insert(particles_[0]);
}

void GalaxySystem::stepBarnesHut(float dt) {
    /*DWORD t = GetTickCount();

    buildTree();

    for (int i = 1; i < GLX_NUM_PARTICLES; i++)
    {
        bht_->calcForce(particles_[i]);
    }

    for (int i = 1; i < GLX_NUM_PARTICLES; i++)
    {
        particles_[i]->integrate(dt);
    }
*/

//printf("Time: %f sec\n", (float)(GetTickCount() - t) * 0.001f);
}

void GalaxySystem::stepBarnesHutSIMD(float dt, const BarnesHutTree &bht, const std::vector<GalaxySystem>& galaxies) {
    // Считаем силы взаимодействия между звездами и ядром
    for (auto &x : particles_) {
        bht.calcForce(x);
    }

    // Считаем действие на звезды темного гало

    for (const auto& galaxy : galaxies) {
        lpVec3 center = galaxy.center_;

        for (auto &particle : particles_) {
            lpVec3 pos = particle.m_pos;
            pos -= center;
            particle.m_forceAccum.addScaled(galaxy.darkMatter_.getGravityVector(pos), particle.m_mass);
        }
    }

    stepParticles(dt);
    update(dt);
}

void GalaxySystem::stepParticles(float dt) {
    // Интегрирование

    GalaxyParticle *p;
    for (auto &x : particles_) {
        p = &x;
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

            movups[eax], xmm0
            movups[ebx], xmm5
            movups[ecx], xmm2
        }
    }
}

void GalaxySystem::update(float dt) {
    //center_ = particles_[0]->m_pos;


    /*float dmag = 0.01f;

    int off = m_numStars + m_numDusts;

    for (int i = 0; i < m_numH2; i++)
    {
        GalaxyParticle *p = particles_[i + off];

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

float GalaxySystem::randNormal() {
    float v1 = 2.0f * RAND - 1.0f;
    float v2 = 2.0f * RAND - 1.0f;
    float r = v1 * v1 + v2 * v2;

    while (r >= 1.0f || r < 0.0000001f) {
        v1 = 2.0f * RAND - 1.0f;
        v2 = 2.0f * RAND - 1.0f;
        r = v1 * v1 + v2 * v2;
    }

    float fac = sqrtf(-2.0f * logf(r) / r);
    return v1 * fac;
}