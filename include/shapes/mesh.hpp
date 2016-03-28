#ifndef RENDERER_PLANE_HPP
#define RENDERER_PLANE_HPP

#include "base.hpp"
#include "shape.hpp"
#include "../geometry.hpp"

namespace renderer {
	typedef std::vector<Vector3dF> VectorArray;
	typedef std::vector<Vector2dF> UVArray;
	typedef std::vector<uint> UIntArray;

	class Mesh:public Shape {
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
