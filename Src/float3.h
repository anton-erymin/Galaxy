#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

// Вектор в трехмерном пространстве
class float3
{
public:

	// x-компонента вектора
	float m_x;

	// y-компонента вектора
	float m_y;

	// z-компонента вектора
	float m_z;

	float m_pad;


	float3();
	float3(float x, float y, float z);
	float3(float s);
	float3(float *v);

	void		clear();
	void		setTo(float x, float y, float z);

	void		add(const float3 &other);
	float3		addR(const float3 &other) const;
	void		sub(const float3 &other);
	float3		subR(const float3 &other) const;
	void 		scale(float s);
	float3		scaleR(float s) const;
	void		addScaled(const float3 &other, float s);
	float3		addScaledR(const float3 &other, float s) const;

	float		dot(const float3 &other);
	void		cross(const float3 &other);
	float3		crossR(const float3 &other) const;

	float		norm() const;
	float		normSq();
	float		normalize();


	void	operator+=(const float3 &other);

	void	operator-=(const float3 &other);

	float3  operator*(float s) const;
	void	operator*=(float s);

	float3	operator%(const float3 &other) const;
	void	operator%=(const float3 &other);

};

inline float3 operator+(const float3& lhs, const float3& rhs)
{
    return {lhs.m_x + rhs.m_x, lhs.m_y + rhs.m_y, lhs.m_z + rhs.m_z};
}

inline float3 operator-(const float3& lhs, const float3& rhs)
{
    return {lhs.m_x - rhs.m_x, lhs.m_y - rhs.m_y, lhs.m_z - rhs.m_z};
}

inline bool equal(const float3& lhs, const float3& rhs, float eps)
{
    return std::fabsf(lhs.m_x - rhs.m_x) < eps && std::fabsf(lhs.m_y - rhs.m_y) < eps && std::fabsf(lhs.m_z - rhs.m_z) < eps;
}