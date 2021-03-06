#include "stdafx.h"
#include "sphere.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Sphere::Sphere(float radius): r(radius) {
	}
	Sphere::Sphere(Sphere& s) {
		r = s.r;
	}
	Sphere Sphere::operator = (const Sphere& s) {
		r = s.r;
		return *this;
	}
	void Sphere::Init() {
		sqrRadius = r * r;
	}
	int Sphere::Intersect(Ray& ray, IntersectResult* result) {
		Point3dF center = (*o2w)(Point3dF(0,0,0));
		Vector3dF&& v = ray.o - center;
		float a0 = v.LengthSquare() - sqrRadius;
		float DdotV = ray.d.Dot(v);
		if (DdotV <= 0) {
			float discr = DdotV * DdotV - a0;
			if (discr >= 0) {
				float tHit = -DdotV - float(sqrtf(discr));
				Point3dF position = ray.GetPoint(tHit);
				Normal3dF normal = (position - center).Normalize();
				*result = IntersectResult(this, tHit, position, normal);
				return 0;
			}
		}
		result = &IntersectResult::NoHit;
		return 0;
	}

	BBox Sphere::Bound() const {
		return BBox(Vector3dF(-r, -r, -r), Vector3dF(r, r, r));
	}

	BBox Sphere::WorldBound() const {
		return (*o2w)(Bound());
	}
}
