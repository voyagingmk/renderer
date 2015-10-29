#ifndef PLANE_HPP
#define PLANE_HPP

#include "base.hpp"
#include "shape.hpp"

class Plane:public Shape{
    PtrVector m_normal;
    float m_distance;
    PtrVector m_position;
public:
	Plane(PtrVector normal, float distance);
	Plane(const Plane &);
	Plane operator = (const Plane&);
	virtual void init();
	virtual PtrIntersectResult intersect(PtrRay);
	inline PtrVector getNormal() const {return m_normal;}
	inline PtrVector getPosition() const {return m_position;}
	inline float getDistance() const {return m_distance;}
};

#endif // PLANE_HPP
