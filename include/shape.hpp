#ifndef RENDERER_SHAPE_HPP
#define RENDERER_SHAPE_HPP

#include "base.hpp"
#include "transform.hpp"
#include "bbox.hpp"

namespace renderer {
	class Shape {
		//default Normal = (0, 1, 0)
		//default Position = (0, 0, 0)
	public:
		int id;
		Transform4x4* o2w;
		Transform4x4* w2o;
		Material* material;
	public:
		Shape() noexcept :material(nullptr) {};
		virtual ~Shape() noexcept{
			o2w = nullptr;
			w2o = nullptr;
			material = nullptr;
		};
		virtual void Init() = 0;
		virtual int Intersect(Ray&, IntersectResult*) = 0;
		virtual BBox Bound() const = 0;
		virtual BBox WorldBound() const = 0;
	};

	typedef std::vector<Shape*> Shapes;
}

#endif //RENDERER_SHAPE_HPP
