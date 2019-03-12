#include "float3.h"

float3::float3()
{
	clear();
}

float3::float3(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

float3::float3(float s)
{
	m_x = s;
	m_y = s;
	m_z = s;
}

float3::float3(float *v)
{
	m_x = v[0];
	m_y = v[1];
	m_z = v[2];
}

void float3::clear()
{
	m_x   = 0.0f;
	m_y   = 0.0f;
	m_z	  = 0.0f;
}

void float3::setTo(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

void float3::add(const float3& other)
{
	m_x += other.m_x;
	m_y += other.m_y;
	m_z += other.m_z;
}

float3 float3::addR(const float3& other) const
{
	return float3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
}

void float3::sub(const float3& other)
{
	m_x -= other.m_x;
	m_y -= other.m_y;
	m_z -= other.m_z;
}

float3 float3::subR(const float3& other) const
{
	return float3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
}

void float3::scale(float s)
{
	m_x *= s;
	m_y *= s;
	m_z *= s;
}

float3 float3::scaleR(float s) const
{
	return float3(s * m_x, s * m_y, s * m_z);
}

void float3::addScaled(const float3& other, float s)
{
	m_x += s * other.m_x;
	m_y += s * other.m_y;
	m_z += s * other.m_z;
}

float3 float3::addScaledR(const float3& other, float s) const
{
	return float3(m_x + s * other.m_x,	m_y + s * other.m_y, m_z + s * other.m_z);
}

float float3::dot(const float3& other)
{
	return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
}

void float3::cross(const float3& other)
{
	float nx = m_y * other.m_z - m_z * other.m_y;
	float ny = m_z * other.m_x - m_x * other.m_z;
	float nz = m_x * other.m_y - m_y * other.m_x;
	m_x = nx;
	m_y = ny;
	m_z = nz;
}

float3 float3::crossR(const float3& other) const
{
	return float3(m_y * other.m_z - m_z * other.m_y, m_z * other.m_x - m_x * other.m_z, m_x * other.m_y - m_y * other.m_x);
}

float float3::norm() const
{
	return sqrtf(m_x * m_x + m_y * m_y + m_z * m_z);
}


float float3::normSq()
{
	return m_x * m_x + m_y * m_y + m_z * m_z;
}

float float3::normalize()
{
	float norm = sqrtf(m_x * m_x + m_y * m_y + m_z * m_z);

	if (norm > 0)
	{
		m_x /= norm;
		m_y /= norm;
		m_z /= norm;
	}

	return norm;
}

void float3::operator+=(const float3& other)
{
	m_x += other.m_x;
	m_y += other.m_y;
	m_z += other.m_z;
}

void float3::operator -=(const float3& other)
{
	m_x -= other.m_x;
	m_y -= other.m_y;
	m_z -= other.m_z;
}

float3 float3::operator*(float s) const
{
	return float3(s * m_x, s * m_y, s * m_z);
}

void float3::operator*=(float s)
{
	m_x *= s;
	m_y *= s;
	m_z *= s;
}

float3 float3::operator %(const float3& other) const
{
	return float3(m_y * other.m_z - m_z * other.m_y, m_z * other.m_x - m_x * other.m_z, m_x * other.m_y - m_y * other.m_x);
}

void float3::operator %=(const float3& other)
{
	float nx = m_y * other.m_z - m_z * other.m_y;
	float ny = m_z * other.m_x - m_x * other.m_z;
	float nz = m_x * other.m_y - m_y * other.m_x;
	m_x = nx;
	m_y = ny;
	m_z = nz;
}