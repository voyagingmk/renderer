#ifndef RENDERER_SPHERE_HPP
#define RENDERER_SPHERE_HPP

#include "base.hpp"
#include "shape.hpp"
namespace renderer {
	class Sphere :public Shape {
		PtrVector center;
		float r;
		float sqrRadius;
	public:
		Sphere(PtrVector center, float radius);
		Sphere(Sphere &);
		Sphere operator = (const Sphere&);
		virtual void Init();
		virtual PtrIntersectResult Intersect(PtrRay);
	};
}

#endif // RENDERER_SPHERE_HPP
