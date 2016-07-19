#ifndef RENDERER_SHAPE_HPP
#define RENDERER_SHAPE_HPP

#include "base.hpp"
#include "transform.hpp"
#include "bbox.hpp"

namespace renderer {
	class Shape {
	public:
		Transform4x4 objSpace;
		Material* material;
	public:
		Shape() noexcept :material(nullptr) {};
		virtual void Init() = 0;
		virtual int Intersect(Ray&, IntersectResult*) = 0;
		virtual BBox Bound() const = 0;
	};

	typedef std::vector<Shape*> Shapes;
}

#endif //RENDERER_SHAPE_HPP
