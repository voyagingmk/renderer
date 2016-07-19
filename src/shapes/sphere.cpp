#include "stdafx.h"
#include "sphere.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Sphere::Sphere(Vector3dF& center, float radius) :center(center), r(radius) {
	}
	Sphere::Sphere(Sphere& s) {
		center = s.center;
		r = s.r;
	}
	Sphere Sphere::operator = (const Sphere& s) {
		r = s.r;
		center = s.center;
		return *this;
	}
	void Sphere::Init() {
		sqrRadius = r * r;
	}
	int Sphere::Intersect(Ray& ray, IntersectResult* result) {
		Vector3dF&& v = ray.o - center;
		float a0 = v.LengthSquare() - sqrRadius;
		float DdotV = ray.d.Dot(v);
		if (DdotV <= 0) {
			float discr = DdotV * DdotV - a0;
			if (discr >= 0) {
				float distance = -DdotV - float(sqrtf(discr));
				Vector3dF position = ray.GetPoint(distance);
				Vector3dF normal = (position - center).Normalize();
				*result = IntersectResult(this, distance, position, normal);
				return 0;
			}
		}
		result = &IntersectResult::NoHit;
		return 0;
	}

	BBox Sphere::Bound() const {
		return BBox(Vector3dF(-r, -r, -r), Vector3dF(r, r, r));
	}

	BBox Mesh::WorldBound() const {
		return (*o2w)(Bound());
	}
}
