#include "stdafx.h"
#include "plane.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Plane::Plane(Vector3dF& normal, float distance) :
		normal(normal),
		distance(distance) {

	};

	Plane::Plane(const Plane& p) {
		normal = p.normal;
		distance = p.distance;
	}
	Plane Plane::operator = (const Plane& p) {
		normal = p.normal;
		distance = p.distance;
		return *this;
	}
	void Plane::Init() {
		position = normal * distance;
	}
	int Plane::Intersect(Ray& ray, IntersectResult* result) {
		float dotA = ray.d.Dot(normal);
		if (dotA >= 0) {
			result = &IntersectResult::NoHit;
			return 0;
		}
		float dotB = normal.Dot(ray.o - position);
		float distance = -dotB / dotA;
		*result = IntersectResult(this, distance, ray.GetPoint(distance), normal);
		return 0;
	}	
	
	BBox Plane::Bound() const {
		return BBox(Vector3dF(0, 0, 0), Vector3dF(1, 1, 1));
	}

	BBox Plane::WorldBound() const {
		return (*o2w)(Bound());
	}
}
