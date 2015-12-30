#include "stdafx.h"
#include "intersect_result.hpp"
#include "geometry.hpp"

namespace renderer {
	IntersectResult::IntersectResult() :
		geometry(nullptr),
		distance(0),
		position(Const::Zero),
		normal(Const::Zero) {
	};

	IntersectResult::IntersectResult(Shape* pShape, float dis, Vector3dF& pos, Vector3dF& normal) :
		geometry(pShape),
		distance(dis),
		position(pos),
		normal(normal) {
	};


	IntersectResult IntersectResult::NoHit = IntersectResult();

}