#ifndef RENDERER_SHAPE_HPP
#define RENDERER_SHAPE_HPP

#include "base.hpp"
namespace renderer {

	class Shape : public std::enable_shared_from_this<Shape> {
	public:
		PtrMaterial material;
	public:
		Shape() :material(nullptr) {};
		virtual void Init() = 0;
		virtual int Intersect(Ray&, IntersectResult*) = 0;
	};
}

#endif //RENDERER_SHAPE_HPP
