#ifndef RENDERER_SPHERE_HPP
#define RENDERER_SPHERE_HPP

#include "base.hpp"
#include "shape.hpp"
#include "../geometry.hpp"


namespace renderer {

	class Sphere :public Shape {
		Vector center;
		float r;
		float sqrRadius;
	public:
		Sphere(Vector& center, float radius);
		Sphere(Sphere &);
		Sphere operator = (const Sphere&);
		virtual void Init();
		virtual int Intersect(Ray&, IntersectResult*);
	};
}

#endif // RENDERER_SPHERE_HPP
