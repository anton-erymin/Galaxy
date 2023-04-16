#include "MathUtils.h"

float integrate_rect(float a, float b, int n, float(*f)(float))
{
    float res, h, hh;

    h = (b - a) / n;
    hh = 0.5f * h;

    res = 0.0f;
    for (int i = 0; i < n; i++)
    {
        res += f(a + i * h + hh);
    }
    res *= h;

    return res;
}

float integrate_trap(float a, float b, int n, float(*f)(float))
{
    float res, h;

    h = (b - a) / n;

    res = 0.5f * (f(a) + f(b));

    for (int i = 1; i < n; i++)
    {
        res += f(a + i * h);
    }
    res *= h;

    return res;
}

void Poisson1(uint32_t numIter, float min, float max, int n, float *data, const vector<float>& rightPart)
{
    assert(numIter > 0);
    assert(min < max);
    assert(n > 3);
    assert(data);

    float h = (max - min) / (n - 1);
    float h2 = h * h;
    float nu;
    float relax = 1.2f;

    for (uint32_t iter = 0; iter < numIter; iter++)
    {
        for (int i = 1; i < n - 1; i++)
        {
            nu = 0.5f * (data[i - 1] + data[i + 1] - h2 * rightPart[i]);
            data[i] += relax * (nu - data[i]);
        }
        data[0] = data[1];
        data[n - 1] = data[n - 2];
    }

    data[0] = data[1];
    data[n - 1] = data[n - 2];
}

bool poisson1d(int numIter, float min, float max, int n, float *data, float(*f)(float x))
{
    if (numIter <= 0)	return false;
    if (min >= max)		return false;
    if (n < 3)			return false;
    if (!data)			return false;
    if (!f)				return false;

    float h = (max - min) / (n - 1);
    float h2 = h * h;
    float nu;
    float relax = 1.5f;

    for (int iter = 0; iter < numIter; iter++)
    {
        for (int i = 1; i < n - 1; i++)
        {
            nu = 0.5f * (data[i - 1] + data[i + 1] - h2 * f(min + i * h));
            data[i] += relax * (nu - data[i]);
        }
    }

    return true;
}

bool poisson2d(int numIter, float min, float max, int n, float **data, float(*f)(float x, float y))
{
    if (numIter <= 0)	return false;
    if (min >= max)		return false;
    if (n < 3)			return false;
    if (!data)			return false;
    if (!f)				return false;

    float h = (max - min) / (n - 1);
    float h2 = h * h;
    float nu;
    float relax = 1.3f;

    for (int iter = 0; iter < numIter; iter++)
    {
        for (int i = 1; i < n - 1; i++)
        {
            for (int j = 1; j < n - 1; j++)
            {
                nu = 0.25f * (data[i - 1][j] + data[i + 1][j] + data[i][j - 1] + data[i][j + 1] - h2 * f(min + i * h, min + j * h));
                data[i][j] += relax * (nu - data[i][j]);
            }
        }
    }

    return true;
}

bool poisson3d(int numIter, float min, float max, int n, float ***data, float(*f)(float x, float y, float z))
{
    if (numIter <= 0)	return false;
    if (min >= max)		return false;
    if (n < 3)			return false;
    if (!data)			return false;
    if (!f)				return false;

    float h = (max - min) / (n - 1);
    float h2 = h * h;
    float nu;
    float relax = 1.3f;
    float factor = 1.0f / 6.0f;

    for (int iter = 0; iter < numIter; iter++)
    {
        for (int i = 1; i < n - 1; i++)
        {
            for (int j = 1; j < n - 1; j++)
            {
                for (int k = 1; k < n - 1; k++)
                {
                    nu = data[i - 1][j][k] + data[i + 1][j][k] +
                        data[i][j - 1][k] + data[i][j + 1][k] +
                        data[i][j][k - 1] + data[i][j][k + 1];
                    nu -= h2 * f(min + i * h, min + j * h, min + k * h);
                    nu *= factor;
                    data[i][j][k] += relax * (nu - data[i][j][k]);
                }
            }
        }
    }

    return true;
}

float RandomStandardDistribution()
{
    float v1 = 2.0f * RAND_NORM - 1.0f;
    float v2 = 2.0f * RAND_NORM - 1.0f;
    float r = v1 * v1 + v2 * v2;

    while (r >= 1.0f || r < 0.0000001f)
    {
        v1 = 2.0f * RAND_NORM - 1.0f;
        v2 = 2.0f * RAND_NORM - 1.0f;
        r = v1 * v1 + v2 * v2;
    }

    float fac = sqrtf(-2.0f * logf(r) / r);
    return v1 * fac;
}

float3 SphericalToCartesian(float r, float phi, float theta)
{
    return { r * sinf(theta) * cosf(phi), r * sinf(theta) * sinf(phi), r * cosf(theta) };
}

float3 SphericalToCartesian(const float3& spherical)
{
    return SphericalToCartesian(spherical.x, spherical.y, spherical.z);
}

float3 CylindricalToCartesian(float r, float phi, float z)
{
    return { r * cosf(phi), r * sinf(phi), z };
}

float3 CylindricalToCartesian(const float3 & cylindrical)
{
    return CylindricalToCartesian(cylindrical.x, cylindrical.y, cylindrical.z);
}

float3 RandomUniformSpherical(float rmin, float rmax)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * PI * RAND_NORM /*- PI*/, 2.0f * PI * RAND_NORM };
}

float3 RandomUniformCylindrical(float rmin, float rmax, float height)
{
    return { RAND_RANGE(rmin, rmax), 2.0f * PI * RAND_NORM, RAND_RANGE(-0.5f * height, 0.5f * height) };
}

float SoftenedDistance(float dist_squared, float soft_factor)
{
    return sqrtf(dist_squared + soft_factor * soft_factor);
}

float3 GravityAcceleration(const float3& l, float mass, float soft, float length_sq)
{
    float3 acceleration = l;
    float distance_sq = length_sq > 0.0f ? length_sq : dot(l, l);
    float r = sqrtf(distance_sq + soft * soft);
    float denom = r * r * r;
    acceleration *= mass / denom;
    return acceleration;
}

float3 GravityAcceleration(const float3& l, float mass, float softened_dist_cubic)
{
    return l * mass / softened_dist_cubic;
}

/** Radial velocity about body with certain mass at distance r. */
float RadialVelocity(float mass, float r)
{
    return sqrtf(mass / r);
}

/** Radial velocity of body with mass in force field at distance r. */
float RadialVelocity(float force, float mass, float r)
{
    return sqrtf(force * r / mass);
}

float PseudoIsothermal(float r, float rho0, float radius)
{
    return rho0 / (1.0f + (r / radius) * (r / radius));
}

float PlummerDensity(float r, float mass, float radius)
{
    return (3.0f * mass / (4.0f * PI * radius * radius * radius)) *
        (1.0f / sqrtf(pow((1.0f + (r * r) / (radius * radius)), 5.0f)));
}

float PlummerPotential(float r, float mass, float radius)
{
    return -mass / (sqrtf(r * r + radius * radius));
}

void IntegrateMotionEquation(float time, float3& position, float3& velocity, 
    const float3& force, float inverse_mass)
{
    // Euler-Cromer
    /*float3 a = acceleration + force * inverseMass;
    velocity += acceleration * time;
    position += velocity * time;*/
    //float3 acceleration = inverse_mass * force;
    velocity += time * force;
    position += time * velocity;
}
