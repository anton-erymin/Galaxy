#pragma once


#define _USE_MATH_DEFINES
#include <math.h>


// Вектор в трехмерном пространстве
class lpVec3
{
public:

	// x-компонента вектора
	float m_x;

	// y-компонента вектора
	float m_y;

	// z-компонента вектора
	float m_z;

	float m_pad;


	lpVec3();
	lpVec3(float x, float y, float z);
	lpVec3(float s);
	lpVec3(float *v);

	void		clear();
	void		setTo(float x, float y, float z);

	void		add(const lpVec3 &other);
	lpVec3		addR(const lpVec3 &other);
	void		sub(const lpVec3 &other);
	lpVec3		subR(const lpVec3 &other);
	void 		scale(float s);
	lpVec3		scaleR(float s);
	void		addScaled(const lpVec3 &other, float s);
	lpVec3		addScaledR(const lpVec3 &other, float s);

	float		dot(const lpVec3 &other);
	void		cross(const lpVec3 &other);
	lpVec3		crossR(const lpVec3 &other);

	float		norm();
	float		normSq();
	float		normalize();

	lpVec3	operator+(const lpVec3 &other);
	void	operator+=(const lpVec3 &other);

	lpVec3	operator-(const lpVec3 &other);
	void	operator-=(const lpVec3 &other);

	float   operator*(const lpVec3 &other);
	lpVec3  operator*(float s);
	void	operator*=(float s);

	lpVec3	operator%(const lpVec3 &other);
	void	operator%=(const lpVec3 &other);

};