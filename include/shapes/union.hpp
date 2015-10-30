#ifndef RENDERER_UNION_HPP
#define RENDERER_UNION_HPP

#include "base.hpp"
#include "shape.hpp"


namespace renderer {
	class Union :public Shape {
	public:
		std::vector<PtrShape> geometries;
	public:
		Union(std::vector<PtrShape>);
		void Init();
		int Intersect(Ray&, IntersectResult*);
	};
}


#endif // RENDERER_UNION_HPP
