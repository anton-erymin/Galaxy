#pragma once

class Galaxy;

struct Particle
{
    bool active = true;
    float activationTime = 0;
    float timer = 0;

    float3 position = {};
    float3 linearVelocity = {};
    float3 acceleration = {};
    float3 force = {};

    float mass = 1.0f;
    float inverseMass = 1.0f;

    bool movable = true;

    float m_alpha;
    float3 color = { 1.0f, 1.0f, 1.0f };

    float magnitude = 1.0f;
    float size = 1.0f;

    bool doubleDrawing = false;

    int	userData = 0;

    Galaxy* galaxy = nullptr;

    void SetMass(float mass);
};
