#include "stdafx.h"
#include "plane.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Plane::Plane(Vector& normal, float distance) :
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
		*result = IntersectResult(shared_from_this(), distance, ray.GetPoint(distance), normal);
		return 0;
	}
}
