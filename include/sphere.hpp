#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "base.hpp"
#include "shape.hpp"

class Sphere:public Shape{
    PtrVector m_center;
    float m_radius;
    float m_sqrRadius;
public:
	Sphere(PtrVector center, float radius);
	Sphere(Sphere &);
	Sphere operator = (const Sphere&);
	virtual void init();
	virtual PtrIntersectResult intersect(PtrRay);
	PtrVector getCenter() const {return m_center;}
	inline float getRadius() const {return m_radius;}
};

#endif // SPHERE_HPP
