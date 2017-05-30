#include "stdafx.h"
#include "mesh.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"
#include "defines.hpp"
#include "union.hpp"

namespace renderer {

	Triangle::Triangle(const Triangle& tri) {
		mesh = tri.mesh;
		indexes[0] = tri.indexes[0];
		indexes[1] = tri.indexes[1];
		indexes[2] = tri.indexes[2];
	}

	Triangle::~Triangle() {
		mesh = nullptr;
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
		const Vertices& vertices = mesh->vertices;
		const Vertex& v0 = vertices[indexes[0]];
		const Vertex& v1 = vertices[indexes[1]];
		const Vertex& v2 = vertices[indexes[2]];

		const Vector3dF& p0 = v0.position;
		const Vector3dF& p1 = v1.position;
		const Vector3dF& p2 = v2.position;
		const Vector3dF&& e1 = p1 - p0;
		const Vector3dF&& e2 = p2 - p0;
		if (e1.isEmpty() || e2.isEmpty()) {
			return 0;
		}
		Normal3dF faceNormal = (e1.Cross(e2)).Normalize();
		if (mesh->reverse) {
			faceNormal = -faceNormal;
		}
		float nDotRay = faceNormal.Dot(ray.d);
		logDebug("tri_idx:%d, v:%d,%d,%d\n", tri_idx, indexes[0], indexes[1], indexes[2]);
		logDebug("nDotRay %.1f mesh->face %d ray.d: %.2f,%.2f,%.2f len:%.1f  n: %.1f,%.1f,%.1f\n",
			nDotRay, mesh->face,
			ray.d.x, ray.d.y, ray.d.z, ray.d.Length(), faceNormal.x, faceNormal.y, faceNormal.z);
		if (mesh->face == 0) { //only front face
			if (nDotRay >= 0) {
				return 0;
			}
		}
		else if (mesh->face == 1) { //only back face
			if (nDotRay <= 0) {
				return 0;
			}
		}
		const Vector3dF& s = ray.o - p0;
		const Vector3dF& s1 = ray.d.Cross(e2);
		Real d = s1.Dot(e1);
		if (almost_equal(d, Real(0), 2))
			return 0;
		d = 1. / d;
		const Vector3dF& s2 = s.Cross(e1);
		const Real& r1 = s2.Dot(e2);
		const Real& r2 = s1.Dot(s);
		const Real& r3 = s2.Dot(ray.d);
		const Real& t = r1 * d;
		const Real& b1 = r2 * d;
		if (b1 < 0. || b1 > 1.)
			return 0;
		const Real& b2 = r3 * d;
		if (b2 < 0. || b1 + b2 > 1.)
			return 0;
		const Vector3dF&& position = ray.GetPoint(t);
		// interpolated normal
		Normal3dF normal = (1 - b1 - b2) * v0.normal + 
			b1 * v1.normal + 
			b2 * v2.normal;
		*result = IntersectResult(mesh, t, std::forward<const Vector3dF>(position), normal.Normalize());
		return 0;
	}

	BBox Triangle::Bound() const {
		const Vertices& vertices = mesh->vertices;
		const Vector3dF& p0 = vertices[indexes[0]].position;
		const Vector3dF& p1 = vertices[indexes[1]].position;
		const Vector3dF& p2 = vertices[indexes[2]].position;
		return BBox(p0, p1).Union(p2);
	}

	BBox Triangle::WorldBound() const {
		return (*o2w)(Bound());
	}

	Mesh::Mesh(Vertices& v, UIntArray& i) {
		vertices = v;
		indexes = i;
	}

	Mesh::Mesh(const Mesh& m) {
		vertices = m.vertices;
		indexes = m.indexes;
	}

	Mesh::~Mesh() {
		vertices.clear();
		indexes.clear();
	}

	Mesh Mesh::operator = (const Mesh& m) {
		vertices = m.vertices;
		indexes = m.indexes;
		return *this;
	}

	void Mesh::Init() {
		initBound();
		initVertexNormals();
	}

	int Mesh::Intersect(Ray& ray, IntersectResult* result) {
		logDebug("Mesh.Intersect: %f, %f\n", ray.o.x, ray.o.y);
		Ray r = (*w2o)(ray);
		float minDistance = FLOAT_MAX;
		float hitt0, hitt1;
		if (!bbox.Intersect(r, &hitt0, &hitt1))
			return 0;
		Triangle tri(this);
		for (int tri_idx = 0, tri_num = indexes.size()/3; tri_idx < tri_num; tri_idx += 1) {
			tri.tri_idx = tri_idx;
			tri.indexes[0] = indexes[tri_idx * 3];
			tri.indexes[1] = indexes[tri_idx * 3 + 1];
			tri.indexes[2] = indexes[tri_idx * 3 + 2];
			if (!tri.Bound().Intersect(r, &hitt0, &hitt1)) {
				continue;
			}
			IntersectResult resultTmp;
			tri.Intersect(r, &resultTmp);
			if (resultTmp.geometry && resultTmp.tHit < minDistance) {
				minDistance = resultTmp.tHit;
				*result = resultTmp;
				logDebug("%d, ray:%f,%f,%f min: %f\n", tri_idx,
					ray.d.x, ray.d.y, ray.d.z, minDistance);
			}
		}
		if (result->geometry) {
			result->normal = (*o2w)(result->normal);
			result->position = (*o2w)(result->position);
		}
		return 0;
	}

	void Mesh::initBound() {
		if (bbox.IsEmpty()) {
			Triangle tri(this);
			for (int tri_idx = 0, tri_num = indexes.size() / 3; tri_idx < tri_num; tri_idx += 1) {
				tri.indexes[0] = indexes[tri_idx * 3];
				tri.indexes[1] = indexes[tri_idx * 3 + 1];
				tri.indexes[2] = indexes[tri_idx * 3 + 2];
				bbox = Union(bbox, tri.Bound());
			}
		}
	}
	void Mesh::initVertexNormals() {
        const Normal3dF emptyNormal = Normal3dF(0.f, 0.f, 0.f);
		for (int vIdx = 0; vIdx < vertices.size(); vIdx++) {
            Vertex& v = vertices[vIdx];
            v.normal = emptyNormal;
		}
		for (int tri_idx = 0, tri_num = indexes.size() / 3; tri_idx < tri_num; tri_idx += 1) {
			int vIdxes[3];
			vIdxes[0] = indexes[tri_idx * 3];
			vIdxes[1] = indexes[tri_idx * 3 + 1];
			vIdxes[2] = indexes[tri_idx * 3 + 2];
            Vertex& v0 = vertices[vIdxes[0]];
            Vertex& v1 = vertices[vIdxes[1]];
            Vertex& v2 = vertices[vIdxes[2]];
			const Vector3dF& p0 = v0.position;
			const Vector3dF& p1 = v1.position;
			const Vector3dF& p2 = v2.position;
			const Vector3dF&& e1 = p1 - p0;
			const Vector3dF&& e2 = p2 - p0;
			const Vector3dF&& faceNormal = (e1.Cross(e2));
			v0.normal += faceNormal;
			v1.normal += faceNormal;
			v2.normal += faceNormal;
		}
		for (int vIdx = 0; vIdx < vertices.size(); vIdx++) {
            Vertex& v = vertices[vIdx];
			v.normal = v.normal.Normalize();
			if (reverse) {
				v.normal = -v.normal;
			}
			}
	}

	BBox Mesh::Bound() const {
		return bbox;
	}

	BBox Mesh::WorldBound() const {
		return (*o2w)(bbox);
	}
}
