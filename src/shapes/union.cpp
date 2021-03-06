#include "stdafx.h"
#include "union.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"
#include "geometry.hpp"
#include "material.hpp"

namespace renderer {

	ShapeUnion::ShapeUnion(std::vector<Shape*> geometries) :geometries(geometries) {
	}
	ShapeUnion::ShapeUnion() {
	}
	void ShapeUnion::Init() {
		for (auto p : geometries) {
			p->Init();
			bbox = Union(bbox, p->Bound());
		}
	}
	int ShapeUnion::Intersect(Ray& ray, IntersectResult* minResult) {
		float minDistance = FLOAT_MAX;
		*minResult = IntersectResult::NoHit;
		for (auto p : geometries) {
			IntersectResult resultTmp;
			p->Intersect(ray, &resultTmp);
			if (resultTmp.geometry && resultTmp.tHit < minDistance) {
				minDistance = resultTmp.tHit;
				*minResult = resultTmp;
				//printf("%f,%f,%f == %f,%f\n",ray->getDirection()->x(),ray->getDirection()->y(),ray->getDirection()->z(),minDistance, result->getGeometry()->getMaterial()->getReflectiveness());
			}
		}
		return 0;
	}

	BBox ShapeUnion::Bound() const {
		return bbox;
	}

	BBox ShapeUnion::WorldBound() const {
		return (*o2w)(Bound());
	}
}