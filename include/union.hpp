#ifndef RENDERER_UNION_HPP
#define RENDERER_UNION_HPP

#include "base.hpp"
#include "shape.hpp"


namespace renderer {
	class Union :public Shape {
	public:
		std::vector<PtShape> geometries;
	public:
		Union(std::vector<PtShape>);
		void Init();
		PtrIntersectResult Intersect(PtrRay ray);
	};
}


#endif // RENDERER_UNION_HPP
