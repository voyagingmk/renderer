#ifndef RENDERER_MESH_HPP
#define RENDERER_MESH_HPP

#include "base.hpp"
#include "shape.hpp"
#include "../geometry.hpp"

namespace renderer {
	class Mesh;
	
	class Triangle : public Shape {
	public:
		Mesh* mesh;
		int indexes[3];
	public:
		Triangle(Mesh* m):mesh(m), indexes{ 0,0,0 } {};
		Triangle(const Triangle&);
		Triangle operator = (const Triangle&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;		
		virtual BBox Bound() const override; 
		virtual BBox WorldBound() const override;
	};

	class Mesh: public Shape {
	public:
		VectorArray vertices;
		NormalArray normals;
		UIntArray indexes;
		UVArray uvs;
		BBox bbox;
	public:
		Mesh(VectorArray& v, NormalArray& n, UIntArray& i, UVArray& uv):vertices(v),normals(n),indexes(i),uvs(uv){};
		Mesh(const Mesh&);
		Mesh operator = (const Mesh&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		void initBound();
		virtual BBox Bound() const override;
		virtual BBox WorldBound() const override;
	};

}
#endif // RENDERER_PLANE_HPP
