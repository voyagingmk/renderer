#ifndef RENDERER_INTERSECT_RESULT_HPP
#define RENDERER_INTERSECT_RESULT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {

	class IntersectResult {
	public:
		Shape* geometry;
		float distance;
		Point3dF position;
		Normal3dF normal;
		static IntersectResult NoHit;
	public:
		IntersectResult();
		IntersectResult(Shape* pShape, float dis, Point3dF& pos, Normal3dF& normal);
	};
}

#endif // RENDERER_INTERSECT_RESULT_HPP
