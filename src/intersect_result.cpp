#include "stdafx.h"
#include "intersect_result.hpp"
#include "geometry.hpp"

namespace renderer {
	IntersectResult::IntersectResult() :
		geometry(nullptr),
		tHit(0),
		position(Const::Zero) {
	};

	IntersectResult::IntersectResult(Shape* pShape, const float t, const Point3dF& pos, const Normal3dF& normal) :
		geometry(pShape),
		tHit(t),
		position(pos),
		normal(normal) {
	};


	IntersectResult IntersectResult::NoHit = IntersectResult();

}