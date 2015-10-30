#include "union.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"
#include "geometry.hpp"
#include "material.hpp"

namespace renderer {

	Union::Union(std::vector<PtrShape> geometries) :geometries(geometries) {
	};

	void Union::Init() {
		for (auto p : geometries) {
			p->Init();
		}
	};
	int Union::Intersect(Ray& ray, IntersectResult* minResult) {
		float minDistance = FLOAT_MAX;
		*minResult = IntersectResult::NoHit;
		IntersectResult resultTmp;
		for (auto p : geometries) {
			p->Intersect(ray, &resultTmp);
			if (resultTmp.geometry && resultTmp.distance < minDistance) {
				minDistance = resultTmp.distance;
				*minResult = resultTmp;
				//printf("%f,%f,%f == %f,%f\n",ray->getDirection()->x(),ray->getDirection()->y(),ray->getDirection()->z(),minDistance, result->getGeometry()->getMaterial()->getReflectiveness());
			}
		}
		return 0;
	};

}