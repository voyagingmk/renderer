#include "stdafx.h"
#include "mesh.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"

namespace renderer {

	Triangle::Triangle(const Triangle& tri) {
		mesh = tri.mesh;
		indexes[0] = tri.indexes[0];
		indexes[1] = tri.indexes[1];
		indexes[2] = tri.indexes[2];
	}

	Triangle Triangle::operator = (const Triangle& tri) {
		mesh = tri.mesh;
		indexes[0] = tri.indexes[0];
		indexes[1] = tri.indexes[1];
		indexes[2] = tri.indexes[2];
		return *this;
	}

	void Triangle::Init() {
	
	}

	//http://www.qiujiawei.com/triangle-intersect/
	int Triangle::Intersect(Ray& ray, IntersectResult* result) {
		auto vertices = mesh->vertices;
		Vector3dF p0 = vertices[indexes[0]];
		Vector3dF p1 = vertices[indexes[1]];
		Vector3dF p2 = vertices[indexes[2]];
		Vector3dF e1 = p1 - p0;
		Vector3dF e2 = p2 - p0;
		Vector3dF s = ray.o - p0;
		Vector3dF s1 = ray.d.Cross(e2);
		Real d = s1.Dot(e1);
		if (almost_equal(d, Real(0), 2))
			return 0;
		d = 1. / d;
		Vector3dF s2 = s.Cross(e1);
		Real r1 = s2.Dot(e2);
		Real r2 = s1.Dot(s);
		Real r3 = s2.Dot(ray.d);
		Real t = r1 * d;
		Real b1 = r2 * d;
		if (b1 < 0. || b1 > 1.)
			return 0;
		Real b2 = r3 * d;
		if (b2 < 0. || b1 + b2 > 1.)
			return 0;
		Vector3dF position = ray.GetPoint(t);
		Vector3dF normal = (e1.Cross(e2)).Normalize();
		if (normal.Dot(ray.d) >= 0)
			normal = -normal;
		*result = IntersectResult(mesh, t, position, normal);
		return 0;
	}

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
		float minDistance = FLOAT_MAX;
		
		Triangle tri(this);
		for (int tri_idx = 0, tri_num = indexes.size()/3; tri_idx < tri_num; tri_idx += 1) {
			tri.indexes[0] = indexes[tri_idx * 3];
			tri.indexes[1] = indexes[tri_idx * 3 + 1];
			tri.indexes[2] = indexes[tri_idx * 3 + 2];
			IntersectResult resultTmp;
			tri.Intersect(ray, &resultTmp);
			if (resultTmp.geometry && resultTmp.distance < minDistance) {
				minDistance = resultTmp.distance;
				*result = resultTmp;
				//printf("%f,%f,%f == %f,%f\n",ray->getDirection()->x(),ray->getDirection()->y(),ray->getDirection()->z(),minDistance, result->getGeometry()->getMaterial()->getReflectiveness());
			}
		}
		return 0;
	}
}
