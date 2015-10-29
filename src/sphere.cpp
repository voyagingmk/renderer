#include "sphere.hpp"
#include "vector.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Sphere::Sphere(PtrVector center, float radius) :m_center(center), r(radius) {
	}
	Sphere::Sphere(Sphere& s) {
		m_center = std::make_shared<Vector>(*s.getCenter());
		r = s.getRadius();
	}
	Sphere Sphere::operator = (const Sphere& s) {
		r = s.getRadius();
		m_center = std::make_shared<Vector>(*s.getCenter());
		return *this;
	}
	void Sphere::init() {
		m_sqrRadius = r * r;
	}
	PtrIntersectResult Sphere::intersect(PtrRay ray) {
		Vector&& v = *(ray->getOrigin()) - (*m_center);
		float a0 = v.squareLength() - m_sqrRadius;
		float DdotV = ray->getDirection()->dot(v);
		if (DdotV <= 0) {
			float discr = DdotV * DdotV - a0;
			if (discr >= 0) {
				auto distance = -DdotV - float(sqrtf(discr));
				auto position = ray->getPoint(distance);
				auto normal = (*position - *m_center).normalize();
				return std::make_shared<IntersectResult>(IntersectResult(shared_from_this(), distance, *position, normal));
			}
		}

		return IntersectResult::NoHit;
	}

}