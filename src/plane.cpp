#include "plane.hpp"
#include "vector.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Plane::Plane(PtrVector normal, float distance) :
		m_normal(normal),
		m_distance(distance) {

	};

	Plane::Plane(const Plane& p) {
		m_normal = p.getNormal();
		m_distance = p.getDistance();
	}
	Plane Plane::operator = (const Plane& p) {
		m_normal = p.getNormal();
		m_distance = p.getDistance();
		return *this;
	}
	void Plane::init() {
		m_position = std::make_shared<Vector>(*m_normal * m_distance);
	}
	PtrIntersectResult Plane::intersect(PtrRay ray) {
		float dotA = ray->getDirection()->dot(*m_normal);
		if (dotA >= 0)
			return IntersectResult::NoHit;
		float dotB = m_normal->dot(*(ray->getOrigin()) - *m_position);
		float distance = -dotB / dotA;
		return std::make_shared<IntersectResult>(IntersectResult(shared_from_this(), distance, ray->getPoint(distance), m_normal));
	}
}
