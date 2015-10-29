#ifndef RENDERER_SPHERE_HPP
#define RENDERER_SPHERE_HPP

#include "base.hpp"
#include "shape.hpp"
namespace renderer {
	class Sphere :public Shape {
		PtrVector m_center;
		float r;
		float m_sqrRadius;
	public:
		Sphere(PtrVector center, float radius);
		Sphere(Sphere &);
		Sphere operator = (const Sphere&);
		virtual void init();
		virtual PtrIntersectResult intersect(PtrRay);
		PtrVector getCenter() const { return m_center; }
		inline float getRadius() const { return r; }
	};
}

#endif // RENDERER_SPHERE_HPP
