#ifndef RENDERER_INTERSECT_RESULT_HPP
#define RENDERER_INTERSECT_RESULT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {

	class IntersectResult {
	public:
		Shape* geometry;
		float distance;
		Vector3dF position;
		Vector3dF normal;
		static IntersectResult NoHit;
	public:
		IntersectResult();
		IntersectResult(Shape* pShape, float dis, Vector3dF& pos, Vector3dF& normal);
	};
}

#endif // RENDERER_INTERSECT_RESULT_HPP
