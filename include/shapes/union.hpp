#ifndef RENDERER_UNION_HPP
#define RENDERER_UNION_HPP

#include "base.hpp"
#include "shape.hpp"


namespace renderer {
	class ShapeUnion :public Shape {
	public:
		Shapes geometries;
		BBox bbox;
	public:
		ShapeUnion(std::vector<Shape*>);
		ShapeUnion();
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		virtual BBox Bound() const override;
	};
}


#endif // RENDERER_UNION_HPP
