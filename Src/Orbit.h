#pragma once

#include "float3.h"

#include <GL/freeglut.h>

class Orbit
{
public:
    void Transform()
    {
        glTranslatef(0.0f, 0.0f, -distance);
        glRotatef(phi, 1.0f, 0.0f, 0.0f);
        glRotatef(theta, 0.0f, 1.0f, 0.0f);
        glTranslatef(-center.m_x, -center.m_y, -center.m_z);

        float modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        right.m_x = modelview[0];
        right.m_y = modelview[4];
        right.m_z = modelview[8];
        up.m_x = modelview[1];
        up.m_y = modelview[5];
        up.m_z = modelview[9];
        forward.m_x = modelview[2];
        forward.m_y = modelview[6];
        forward.m_z = modelview[10];
    }

    void Rotate(float x, float y)
    {
        thetaTarget += x;
        phiTarget += y;
    }

    void MoveForward(float dist)
    {
        distanceTarget = std::max(0.00001f, distanceTarget + dist);
    }

    void Pan(float x, float y)
    {
        center += right * x;
        center += up * y;
    }

    const float& GetDistance() const { return distance; }

    void Update(float time)
    {
        float eps = 0.01f;

        if (fabsf(thetaTarget - theta) > eps)
        {
            theta += (thetaTarget - theta) * 6.0f * time;
        }
        if (fabsf(phiTarget - phi) > eps)
        {
            phi += (phiTarget - phi) * 6.0f * time;
        }
        if (fabsf(distanceTarget - distance) > eps)
        {
            distance += (distanceTarget - distance) * 6.0f * time;
        }
    }

private:
    float3 center;
    float distance = 30.0f;
    float distanceTarget = 30.0f;
    float phi = 0.0f;
    float phiTarget = 0.0f;
    float theta = 0.0f;
    float thetaTarget = 0.0f;
    float3 right;
    float3 up;
    float3 forward;
};
