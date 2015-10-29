#ifndef INTERSECT_RESULT_HPP
#define INTERSECT_RESULT_HPP

#include "base.hpp"

class IntersectResult{
	PtrGeometry m_geometry;
	float m_distance;
	PtrVector m_position;
	PtrVector m_normal;
public:
	IntersectResult();
    IntersectResult(PtrGeometry, float, PtrVector, PtrVector);
    IntersectResult(PtrGeometry, float, const Vector&, const Vector&);
    void setGeometry(PtrGeometry ptrGeo){ m_geometry = ptrGeo;};
    PtrGeometry getGeometry(){ return m_geometry; };
    float getDistance(){ return m_distance; };
    PtrVector getPosition(){ return m_position; };
    PtrVector getNormal(){ return m_normal; };
public:
	static PtrIntersectResult NoHit;
};

#endif // INTERSECT_RESULT_HPP
