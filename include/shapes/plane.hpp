#ifndef RENDERER_PLANE_HPP
#define RENDERER_PLANE_HPP

#include "base.hpp"
#include "shape.hpp"
namespace renderer {
	class Plane :public Shape {
		PtrVector normal;
		float distance;
		PtrVector position;
	public:
		Plane(PtrVector normal, float distance);
		Plane(const Plane &);
		Plane operator = (const Plane&);
		virtual void Init();
		virtual PtrIntersectResult Intersect(PtrRay);
	};

}
#endif // RENDERER_PLANE_HPP
