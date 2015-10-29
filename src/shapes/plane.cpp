#include "plane.hpp"
#include "vector.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Plane::Plane(PtrVector normal, float distance) :
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
		position = std::make_shared<Vector>(*normal * distance);
	}
	PtrIntersectResult Plane::Intersect(PtrRay ray) {
		float dotA = ray->direction->Dot(*normal);
		if (dotA >= 0)
			return IntersectResult::NoHit;
		float dotB = normal->Dot(*(ray->origin) - *position);
		float distance = -dotB / dotA;
		return std::make_shared<IntersectResult>(IntersectResult(shared_from_this(), distance, ray->GetPoint(distance), normal));
	}
}
