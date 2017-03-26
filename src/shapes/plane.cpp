#include "stdafx.h"
#include "plane.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Plane::Plane(float distance) :
		distance(distance) 
	{};

	Plane::Plane(const Plane& p) {
		distance = p.distance;
	}
	Plane Plane::operator = (const Plane& p) {
		distance = p.distance;
		return *this;
	}
	void Plane::Init() {
		position = Vector3dF(0,1,0) * distance;
	}
	int Plane::Intersect(Ray& ray, IntersectResult* result) {
		Normal3dF normal(0, 1, 0);
		float dotA = ray.d.Dot(normal);
		if (dotA >= 0) {
			result = &IntersectResult::NoHit;
			return 0;
		}
		float dotB = normal.Dot(ray.o - position);
		float tHit = -dotB / dotA;
		*result = IntersectResult(this, tHit, ray.GetPoint(tHit), normal);
		return 0;
	}	
	
	BBox Plane::Bound() const {
		return BBox(Vector3dF(0, 0, 0), Vector3dF(1, 1, 1));
	}

	BBox Plane::WorldBound() const {
		return (*o2w)(Bound());
	}
}
