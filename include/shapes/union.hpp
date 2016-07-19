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
		ShapeUnion(const ShapeUnion& s) {
			geometries = s.geometries;
			bbox = s.bbox;
		}
		ShapeUnion& operator=(ShapeUnion&& s) {
			geometries = std::move(s.geometries);
			bbox = std::move(s.bbox);
			return *this;
		}
		ShapeUnion(ShapeUnion&& s) {
			geometries = std::move(s.geometries);
			bbox = std::move(s.bbox);
		}
		ShapeUnion();
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		virtual BBox Bound() const override;
		virtual BBox WorldBound() const override;
	};
}


#endif // RENDERER_UNION_HPP
