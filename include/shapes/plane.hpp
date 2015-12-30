#ifndef RENDERER_PLANE_HPP
#define RENDERER_PLANE_HPP

#include "base.hpp"
#include "shape.hpp"
#include "../geometry.hpp"

namespace renderer {
	class Plane :public Shape {
		Vector3dF normal;
		float distance;
		Vector3dF position;
	public:
		Plane(Vector3dF& normal, float distance);
		Plane(const Plane &);
		Plane operator = (const Plane&);
		virtual void Init();
		virtual int Intersect(Ray&, IntersectResult*);
	};

}
#endif // RENDERER_PLANE_HPP
