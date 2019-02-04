#pragma once

#include "lpVec3.h"

// ���������� ������������ ������������� �������

class SphericalModel {
public:

    // ���-�� ��������� �����
    int		  m_N;
    // ��������� �������
    float	  m_gridXMin;
    float	  m_gridXMax;
    // ��� ���������
    float	  m_h;

    float     m_radius;

    // �������������� ���������
    float	 *m_potential;
    // ������������� ��������������� ����
    float	 *m_gravityField;

    float     m_potentialMax;
    float	  m_potentialMin;

    float	  m_vc;


    SphericalModel(float gridXMin, float gridXMax, float radius);
    ~SphericalModel(void);

    void init();

    // ������ ��������������� ����������
    void calcPotential();
    // ������ �������������
    void calcGravityField();

    lpVec3 getGravityVector(lpVec3 pos) const;
    float  getCircularVelocity(float r);

    void plotPotential() const;


    // ������� ������������� ���������
    static float densityDistribution(float r);
    // ������ ����� ��������� ��������
    static float rightPartPoisson(float r);

};

