#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "GalaxyParticle.h"
#include "lpVec3.h"
#include "BarnesHutTree.h"
#include "SphericalModel.h"


#define		RAND					((float)rand() / RAND_MAX)
#define		RAND_MINMAX(a, b)		(a + (b - a) * RAND)

class GalaxySystem {
public:

    // �����������
    GalaxySystem(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius,
        float haloRadius, float diskThickness, float bulgeMass, float haloMass, float starMass, bool ccw);
    GalaxySystem();

    // �������� ��������� � �� �������������
    void init();

    // ����������� ��������
    void step(float dt);

    // �������� ������-����
    void stepBarnesHut(float dt);

    // �������� ������-���� � SSE ������������
    void stepBarnesHutSIMD(float dt, const BarnesHutTree &bht, const std::vector<GalaxySystem>& galaxies);

    // ���������� ����������
    void addToTree(BarnesHutTree &bht) const;

    // ������� ��������� ���������
    void draw(int mode) const;

    std::vector<GalaxyParticle> &particles() { return particles_; }

private:
    void createBulge();
    void createDisk();

    // ��������� ������ ������
    void drawParticles(lpVec3 v1, lpVec3 v2) const;

    // �������� ������
    void makeStar(GalaxyParticle &particle);
    void makeDust(GalaxyParticle &particle);
    void makeH2(GalaxyParticle &particle);

    // �������������� ������
    void stepParticles(float dt);

    // ���������� ���������� ������
    void update(float dt);

    // ��������� ����� � ���������� ��������������
    float randNormal();

    std::vector<GalaxyParticle> particles_;

    int				 numBulgeStars_;
    float			 bulgeMass_;
    float			 bulgeRadius_;
    int				 numDiskStars_;
    float			 diskRadius_;
    float			 diskThickness_;
    float			 haloRadius_;
    float			 haloMass_;
    float			 starMass_;
    bool			 ccw_;
    lpVec3			 center_;

    SphericalModel   darkMatter_;
};