#ifndef UNION_HPP
#define UNION_HPP

#include "base.hpp"
#include "shape.hpp"


namespace renderer {
	class Union :public Shape {
		std::vector<PtrGeometry> m_geometries;
	public:
		Union(std::vector<PtrGeometry>);
		void init();
		PtrIntersectResult intersect(PtrRay ray);
	};
}


#endif // UNION_HPP
