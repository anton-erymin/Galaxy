#pragma once

class Galaxy;

struct Particle
{
    bool active = true;
    float activationTime = 0;
    float timer = 0;

    Float3 position = {};
    Float3 velocity = {};
    Float3 acceleration = {};
    Float3 force = {};

    float mass = 1.0f;
    float inverse_mass = 1.0f;

    bool movable = true;

    float m_alpha;
    Float3 color = { 1.0f, 1.0f, 1.0f };

    float magnitude = 1.0f;
    float size = 1.0f;
    uint32 texture_idx = 0;

    bool double_drawing = false;

    int	userData = 0;

    Galaxy* galaxy = nullptr;

    void SetMass(float mass);
};
