#include "sphere.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Sphere::Sphere(PtrVector center, float radius) :center(center), r(radius) {
	}
	Sphere::Sphere(Sphere& s) {
		center = std::make_shared<Vector>(*s.center);
		r = s.r;
	}
	Sphere Sphere::operator = (const Sphere& s) {
		r = s.r;
		center = std::make_shared<Vector>(*s.center);
		return *this;
	}
	void Sphere::Init() {
		sqrRadius = r * r;
	}
	PtrIntersectResult Sphere::Intersect(PtrRay ray) {
		Vector&& v = *(ray->origin) - (*center);
		float a0 = v.LengthSquare() - sqrRadius;
		float DdotV = ray->direction->Dot(v);
		if (DdotV <= 0) {
			float discr = DdotV * DdotV - a0;
			if (discr >= 0) {
				auto distance = -DdotV - float(sqrtf(discr));
				auto position = ray->GetPoint(distance);
				auto normal = (*position - *center).Normalize();
				return std::make_shared<IntersectResult>(IntersectResult(shared_from_this(), distance, *position, normal));
			}
		}

		return IntersectResult::NoHit;
	}

}