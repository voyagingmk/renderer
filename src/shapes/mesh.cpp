#include "stdafx.h"
#include "mesh.hpp"

namespace renderer {

	Mesh::Mesh(const Mesh& m) {
		vertices = m.vertices;
		normals = m.normals;
		indexes = m.indexes;
		uvs = m.uvs;
	}

	Mesh Mesh::operator = (const Mesh& m) {
		vertices = m.vertices;
		normals = m.normals;
		indexes = m.indexes;
		uvs = m.uvs;
		return *this;
	}

	void Mesh::Init() {
	}

	int Mesh::Intersect(Ray& ray, IntersectResult* result) {
		return 0;
	}
}
