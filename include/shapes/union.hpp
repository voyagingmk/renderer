#ifndef RENDERER_UNION_HPP
#define RENDERER_UNION_HPP

#include "base.hpp"
#include "shape.hpp"


namespace renderer {
	class ShapeUnion :public Shape {
	public:
		std::vector<Shape*> geometries;
	public:
		ShapeUnion(std::vector<Shape*>);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
	};
}


#endif // RENDERER_UNION_HPP
