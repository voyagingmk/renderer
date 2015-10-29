#include "intersect_result.hpp"
#include "vector.hpp"

namespace renderer {
	IntersectResult::IntersectResult() :
		geometry(nullptr),
		distance(0),
		position(Vector::Zero),
		normal(Vector::Zero) {
	};

	IntersectResult::IntersectResult(PtShape ptrGeo, float dis, PtrVector pos, PtrVector normal) :
		geometry(ptrGeo),
		distance(dis),
		position(pos),
		normal(normal) {
	};

	IntersectResult::IntersectResult(PtShape ptrGeo, float dis, const Vector& pos, const Vector& normal) :
		geometry(ptrGeo),
		distance(dis),
		position(std::make_shared<Vector>(pos)),
		normal(std::make_shared<Vector>(normal)) {
	};

	PtrIntersectResult IntersectResult::NoHit = std::make_shared<IntersectResult>();

}