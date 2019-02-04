#include "lpVec3.h"


lpVec3::lpVec3()
{
	clear();
}


lpVec3::lpVec3(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}


lpVec3::lpVec3(float s)
{
	m_x = s;
	m_y = s;
	m_z = s;
}


lpVec3::lpVec3(float *v)
{
	m_x = v[0];
	m_y = v[1];
	m_z = v[2];
}



void lpVec3::clear()
{
	m_x   = 0.0f;
	m_y   = 0.0f;
	m_z	  = 0.0f;
	m_pad = 0.0f;
}


void lpVec3::setTo(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}


void lpVec3::add(const lpVec3& other)
{
	m_x += other.m_x;
	m_y += other.m_y;
	m_z += other.m_z;
}


lpVec3 lpVec3::addR(const lpVec3& other)
{
	return lpVec3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
}


void lpVec3::sub(const lpVec3& other)
{
	m_x -= other.m_x;
	m_y -= other.m_y;
	m_z -= other.m_z;
}


lpVec3 lpVec3::subR(const lpVec3& other)
{
	return lpVec3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
}


void lpVec3::scale(float s)
{
	m_x *= s;
	m_y *= s;
	m_z *= s;
}


lpVec3 lpVec3::scaleR(float s)
{
	return lpVec3(s * m_x, s * m_y, s * m_z);
}


void lpVec3::addScaled(const lpVec3& other, float s)
{
	m_x += s * other.m_x;
	m_y += s * other.m_y;
	m_z += s * other.m_z;
}


lpVec3 lpVec3::addScaledR(const lpVec3& other, float s)
{
	return lpVec3(m_x + s * other.m_x,	m_y + s * other.m_y, m_z + s * other.m_z);
}



float lpVec3::dot(const lpVec3& other)
{
	return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
}


void lpVec3::cross(const lpVec3& other)
{
	float nx = m_y * other.m_z - m_z * other.m_y;
	float ny = m_z * other.m_x - m_x * other.m_z;
	float nz = m_x * other.m_y - m_y * other.m_x;
	m_x = nx;
	m_y = ny;
	m_z = nz;
}


lpVec3 lpVec3::crossR(const lpVec3& other)
{
	return lpVec3(m_y * other.m_z - m_z * other.m_y, m_z * other.m_x - m_x * other.m_z, m_x * other.m_y - m_y * other.m_x);
}



float lpVec3::norm()
{
	return sqrtf(m_x * m_x + m_y * m_y + m_z * m_z);
}


float lpVec3::normSq()
{
	return m_x * m_x + m_y * m_y + m_z * m_z;
}


float lpVec3::normalize()
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


lpVec3 lpVec3::operator+(const lpVec3& other)
{
	return lpVec3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
}


void lpVec3::operator+=(const lpVec3& other)
{
	m_x += other.m_x;
	m_y += other.m_y;
	m_z += other.m_z;
}


lpVec3 lpVec3::operator-(const lpVec3& other)
{
	return lpVec3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
}


void lpVec3::operator -=(const lpVec3& other)
{
	m_x -= other.m_x;
	m_y -= other.m_y;
	m_z -= other.m_z;
}


float lpVec3::operator*(const lpVec3& other)
{
	return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
}


lpVec3 lpVec3::operator*(float s)
{
	return lpVec3(s * m_x, s * m_y, s * m_z);
}


void lpVec3::operator*=(float s)
{
	m_x *= s;
	m_y *= s;
	m_z *= s;
}


lpVec3 lpVec3::operator %(const lpVec3& other)
{
	return lpVec3(m_y * other.m_z - m_z * other.m_y, m_z * other.m_x - m_x * other.m_z, m_x * other.m_y - m_y * other.m_x);
}


void lpVec3::operator %=(const lpVec3& other)
{
	float nx = m_y * other.m_z - m_z * other.m_y;
	float ny = m_z * other.m_x - m_x * other.m_z;
	float nz = m_x * other.m_y - m_y * other.m_x;
	m_x = nx;
	m_y = ny;
	m_z = nz;
}