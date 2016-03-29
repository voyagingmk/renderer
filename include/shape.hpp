#ifndef RENDERER_SHAPE_HPP
#define RENDERER_SHAPE_HPP

#include "base.hpp"
#include "transform.hpp"

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
	};
}

#endif //RENDERER_SHAPE_HPP
