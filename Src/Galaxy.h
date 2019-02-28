#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "lpVec3.h"
#include "SphericalModel.h"

class Image;

struct Particle
{
    bool active = true;
    float activationTime = 0;
    float timer = 0;

    lpVec3 position = {};
    lpVec3 linearVelocity = {};
    lpVec3 force = {};

    float mass = 1.0f;
    float inverseMass = 1.0f;

    float m_alpha;
    lpVec3 color = { 1.0f, 1.0f, 1.0f };

    float magnitude = 1.0f;
    float size = 1.0f;

    const Image* image;

    bool doubleDrawing = false;

    int	userData = 0;

    void SetMass(float mass);
};

class Galaxy
{
public:
    Galaxy();
    Galaxy(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness,
        float bulgeMass, float haloMass, float starMass, bool ccw);

    void update(float dt);

    std::vector<Particle>& GetParticles() { return particles; }
    const std::unordered_map<const Image*, std::vector<const Particle*>> GetParticlesByImage() const { return image_to_particles; }
    const SphericalModel& GetHalo() const { return halo; }

private:
    void CreateBulge();
    void CreateDisk();

    lpVec3 position;

    std::vector<Particle> particles;
    std::unordered_map<const Image*, std::vector<const Particle*>> image_to_particles;

    int numBulgeStars;
    float bulgeMass;
    float bulgeRadius;

    int numDiskStars;
    float diskRadius;
    float diskThickness;

    float haloRadius;
    float haloMass;

    float starMass;

    bool ccw;

    SphericalModel halo;
};

class Universe
{
public:
    Universe(float size);

    Galaxy& CreateGalaxy();
    Galaxy& CreateGalaxy(lpVec3 center, int numBulgeStars, int numDiskStars, float bulgeRadius, float diskRadius, float haloRadius, float diskThickness,
        float bulgeMass, float haloMass, float starMass, bool ccw);

    float GetSize() const { return size; }
    std::vector<Galaxy>& GetGalaxies() { return galaxies; }

private:
    float size;
    std::vector<Galaxy> galaxies;
};