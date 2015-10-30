#ifndef RENDERER_INTERSECT_RESULT_HPP
#define RENDERER_INTERSECT_RESULT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {

	class IntersectResult {
	public:
		PtrShape geometry;
		float distance;
		Vector position;
		Vector normal;
		static IntersectResult NoHit;
	public:
		IntersectResult();
		IntersectResult(PtrShape pShape, float dis, Vector& pos, Vector& normal);
	};
}

#endif // RENDERER_INTERSECT_RESULT_HPP
