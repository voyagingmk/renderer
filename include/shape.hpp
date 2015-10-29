#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "base.hpp"
namespace renderer {

	class Shape : public std::enable_shared_from_this<Shape> {
		PtrMaterial m_material;
	public:
		Shape() :m_material(nullptr) {};
		virtual void init() = 0;
		virtual PtrIntersectResult intersect(PtrRay) = 0;
		inline void setMaterial(PtrMaterial pMaterial) { m_material = pMaterial; };
		inline PtrMaterial getMaterial() { return m_material; };
	};
}

#endif // GEOMETRY_HPP
