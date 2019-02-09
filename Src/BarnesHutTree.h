//#pragma once
//
//#include "lpVec3.h"
//
//class Particle;
//
//class BarnesHutTree
//{
//public:
//
//	lpVec3				m_point;
//	lpVec3				m_oppositePoint;
//	float				m_length;
//
//	Particle	   *m_particle;
//
//	float				m_totalMass;
//	lpVec3				m_mc;
//
//	bool				m_leaf;
//	BarnesHutTree	  **m_subs;
//
//
//
//
//	BarnesHutTree(const lpVec3 &GetPoint, float length);
//
//	void Insert(Particle *p);
//	void calcForce(Particle *p);
//
//private:
//	bool inline contains(Particle *p);
//
//};


#pragma once

#include <memory>

#include "lpVec3.h"

class Particle;

class BarnesHutTree
{
public:
    BarnesHutTree(const lpVec3 &GetPoint, float length);

    void Insert(const Particle &p);
    lpVec3 CalculateForce(const Particle &particle) const;
    void Reset();

    const lpVec3& GetPoint() const { return point; }
    float GetLength() const { return length; }
    bool IsLeaf() const { return isLeaf; }

    const BarnesHutTree& operator[](size_t i) const { return *children[i]; }

private:
    bool inline contains(const Particle &p) const;

    lpVec3 point;
    lpVec3 oppositePoint;
    float  length;
    float  totalMass;
    lpVec3 massCenter;
    bool   isLeaf;

    std::unique_ptr<BarnesHutTree> children[4];

    const Particle *particle_ = nullptr;

    friend void DrawBarnesHutTree(const BarnesHutTree& node);
};
