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
		int tri_idx;
		int indexes[3];
	public:
		Triangle(Mesh* m):mesh(m), indexes{ 0,0,0 } {};
		~Triangle();
		Triangle(const Triangle&);
		Triangle operator = (const Triangle&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;		
		virtual BBox Bound() const override; 
		virtual BBox WorldBound() const override;
	};

	struct Vertex {
		// Position
		Vector3dF position;
		// Normal
		Normal3dF normal;
        Vector3dF color;
		// TexCoords
		Vector2dF texCoords;
	};
	typedef std::vector<Vertex> Vertices;

	class Mesh: public Shape {
	public:
		Vertices vertices;
		UIntArray indexes;
		BBox bbox;
		int face = 0;
		bool reverse = false;
	public:
		Mesh() {}
		Mesh(Vertices& v, UIntArray& i);
		Mesh(const Mesh&);
		~Mesh();
		Mesh operator = (const Mesh&);
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
		void initBound();
		void initVertexNormals();
		virtual BBox Bound() const override;
		virtual BBox WorldBound() const override;
	};

}
#endif // RENDERER_PLANE_HPP
