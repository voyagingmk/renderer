#include "intersect_result.hpp"
#include "geometry.hpp"

namespace renderer {
	IntersectResult::IntersectResult() :
		geometry(nullptr),
		distance(0),
		position(Vector::Zero),
		normal(Vector::Zero) {
	};

	IntersectResult::IntersectResult(PtrShape pShape, float dis, Vector& pos, Vector& normal) :
		geometry(pShape),
		distance(dis),
		position(pos),
		normal(normal) {
	};


	IntersectResult IntersectResult::NoHit = IntersectResult();

}