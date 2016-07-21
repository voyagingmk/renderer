#include "stdafx.h"
#include "intersect_result.hpp"
#include "geometry.hpp"

namespace renderer {
	IntersectResult::IntersectResult() :
		geometry(nullptr),
		distance(0),
		position(Const::Zero) {
	};

	IntersectResult::IntersectResult(Shape* pShape, float dis, Point3dF& pos, Normal3dF& normal) :
		geometry(pShape),
		distance(dis),
		position(pos),
		normal(normal) {
	};


	IntersectResult IntersectResult::NoHit = IntersectResult();

}