#include "stdafx.h"
#include "mesh.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "intersect_result.hpp"
#include "defines.hpp"

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
		const VectorArray& vertices = mesh->vertices;
		const Vector3dF& p0 = vertices[indexes[0]];
		const Vector3dF& p1 = vertices[indexes[1]];
		const Vector3dF& p2 = vertices[indexes[2]];
		const Vector3dF&& e1 = p1 - p0;
		const Vector3dF&& e2 = p2 - p0;
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
		Normal3dF normal = (1 - b1 - b2) * mesh->vnormals[indexes[0]] + 
			b1 * mesh->vnormals[indexes[1]] + 
			b2 * mesh->vnormals[indexes[2]];
		*result = IntersectResult(mesh, t, std::forward<const Vector3dF>(position), normal);
		return 0;
	}

	BBox Triangle::Bound() const {
		const VectorArray& vertices = mesh->vertices;
		Vector3dF p0 = vertices[indexes[0]];
		Vector3dF p1 = vertices[indexes[1]];
		Vector3dF p2 = vertices[indexes[2]];
		return BBox(p0, p1).Union(p2);
	}

	BBox Triangle::WorldBound() const {
		return (*o2w)(Bound());
	}

	Mesh::Mesh(VectorArray& v, NormalArray& n, UIntArray& i, UVArray& uv) {
		vertices = v;
		vnormals = n;
		indexes = i;
		uvs = uv;
	}

	Mesh::Mesh(const Mesh& m) {
		vertices = m.vertices;
		vnormals = m.vnormals;
		indexes = m.indexes;
		uvs = m.uvs;
	}

	Mesh::~Mesh() {
		vertices.clear();
		vnormals.clear();
		indexes.clear();
		uvs.clear();
	}

	Mesh Mesh::operator = (const Mesh& m) {
		vertices = m.vertices;
		vnormals = m.vnormals;
		indexes = m.indexes;
		uvs = m.uvs;
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
			IntersectResult resultTmp;
			tri.Intersect(r, &resultTmp);
			if (resultTmp.geometry && resultTmp.distance < minDistance) {
				minDistance = resultTmp.distance;
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
		if (vnormals.size() == 0) {
			vnormals.resize(vertices.size());
			for (int tri_idx = 0, tri_num = indexes.size() / 3; tri_idx < tri_num; tri_idx += 1) {
				int vIdxes[3];
				vIdxes[0] = indexes[tri_idx * 3];
				vIdxes[1] = indexes[tri_idx * 3 + 1];
				vIdxes[2] = indexes[tri_idx * 3 + 2];
				const Vector3dF& p0 = vertices[vIdxes[0]];
				const Vector3dF& p1 = vertices[vIdxes[1]];
				const Vector3dF& p2 = vertices[vIdxes[2]];
				const Vector3dF&& e1 = p1 - p0;
				const Vector3dF&& e2 = p2 - p0;
				const Vector3dF&& faceNormal = (e1.Cross(e2)).Normalize();
				vnormals[vIdxes[0]] += faceNormal;
				vnormals[vIdxes[1]] += faceNormal;
				vnormals[vIdxes[2]] += faceNormal;
			}
			for (int vIdx = 0; vIdx < vertices.size(); vIdx++) {
				vnormals[vIdx] = vnormals[vIdx].Normalize();
				if (reverse) {
					vnormals[vIdx] = -vnormals[vIdx];
				}
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
