#include "union.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"
#include "vector.hpp"
#include "material.hpp"

namespace renderer {

	Union::Union(std::vector<PtShape> geometries) :geometries(geometries) {
	};

	void Union::Init() {
		for (auto p : geometries) {
			p->Init();
		}
	};

	PtrIntersectResult Union::Intersect(PtrRay ray) {
		float minDistance = std::numeric_limits<float>::max();
		auto minResult = IntersectResult::NoHit;
		for (auto p : geometries) {
			auto result = p->Intersect(ray);
			if (result->geometry && result->distance < minDistance) {
				minDistance = result->distance;
				minResult = result;
				//printf("%f,%f,%f == %f,%f\n",ray->getDirection()->x(),ray->getDirection()->y(),ray->getDirection()->z(),minDistance, result->getGeometry()->getMaterial()->getReflectiveness());
			}
		}
		return minResult;
	};

}