#ifndef RENDERER_INTERSECT_RESULT_HPP
#define RENDERER_INTERSECT_RESULT_HPP

#include "base.hpp"
namespace renderer {

	class IntersectResult {
	public:
		PtShape geometry;
		float distance;
		PtrVector position;
		PtrVector normal;
		static PtrIntersectResult NoHit;
	public:
		IntersectResult();
		IntersectResult(PtShape, float, PtrVector, PtrVector);
		IntersectResult(PtShape, float, const Vector&, const Vector&);
	};
}

#endif // RENDERER_INTERSECT_RESULT_HPP
