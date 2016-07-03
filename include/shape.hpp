#ifndef RENDERER_SHAPE_HPP
#define RENDERER_SHAPE_HPP

#include "base.hpp"
#include "matrix.hpp"
#include "bbox.hpp"

namespace renderer {
	class Shape {
	public:
		//Matrix4x4 o2w;
		//Matrix4x4 w2o;
		Material* material;
	public:
		Shape() noexcept :material(nullptr) {};
		virtual void Init() = 0;
		virtual int Intersect(Ray&, IntersectResult*) = 0;
		virtual BBox Bound() const = 0;
	};
}

#endif //RENDERER_SHAPE_HPP
