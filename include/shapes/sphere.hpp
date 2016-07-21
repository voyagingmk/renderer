#ifndef RENDERER_SPHERE_HPP
#define RENDERER_SPHERE_HPP

#include "base.hpp"
#include "shape.hpp"
#include "../geometry.hpp"
#include "../bbox.hpp"

namespace renderer {

	class Sphere: public Shape {
		float r;
		float sqrRadius;
	public:
		Sphere(float radius);
		Sphere(Sphere &);
		Sphere operator = (const Sphere&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		virtual BBox Bound() const override;
		virtual BBox WorldBound() const override;
	};
}

#endif // RENDERER_SPHERE_HPP
