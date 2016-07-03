#ifndef RENDERER_PLANE_HPP
#define RENDERER_PLANE_HPP

#include "base.hpp"
#include "shape.hpp"
#include "../geometry.hpp"
#include "bbox.hpp"

namespace renderer {
	class Plane :public Shape {
		Vector3dF normal;
		float distance;
		Vector3dF position;
	public:
		Plane(Vector3dF& normal, float distance);
		Plane(const Plane &);
		Plane operator = (const Plane&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		virtual BBox Bound() const override;
	};

}
#endif // RENDERER_PLANE_HPP
