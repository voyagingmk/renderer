#ifndef RENDERER_PLANE_HPP
#define RENDERER_PLANE_HPP

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
		virtual void Init();
		virtual int Intersect(Ray&, IntersectResult*);
	};

	class Mesh: public Shape {
	public:
		VectorArray vertices;
		VectorArray normals;
		UIntArray indexes;
		UVArray uvs;
	public:
		Mesh(VectorArray& v, VectorArray& n, UIntArray& i, UVArray& uv):vertices(v),normals(n),indexes(i),uvs(uv){};
		Mesh(const Mesh&);
		Mesh operator = (const Mesh&);
		virtual void Init();
		virtual int Intersect(Ray&, IntersectResult*);
	};

}
#endif // RENDERER_PLANE_HPP
