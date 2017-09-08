#include "stdafx.h"
#include "com/bbox.hpp"

namespace renderer {
	BBox::BBox() noexcept
	{
	};

	BBox::BBox(const Vector3dF &p) noexcept:
		pMin(p),
		pMax(p)
	{
		
	};

	BBox::BBox(const Vector3dF& p1, const Vector3dF& p2) noexcept {
		pMin = Vector3dF(min(p1.x, p2.x), min(p1.y, p2.y), min(p1.z, p2.z));
		pMax = Vector3dF(max(p1.x, p2.x), max(p1.y, p2.y), max(p1.z, p2.z));
	};

	BBox::BBox(const BBox& b) noexcept{
		pMin = b.pMin;
		pMax = b.pMax;
	}

	BBox::BBox(BBox&& b) noexcept {
		pMin = b.pMin;
		pMax = b.pMax;
	};

	BBox::~BBox() {};

	BBox& BBox::operator = (const BBox& b) noexcept {
		pMin = b.pMin;
		pMax = b.pMax;
		return *this;
	};

	BBox& BBox::operator = (BBox&& b) noexcept {
		pMin = b.pMin;
		pMax = b.pMax;
		return *this;
	};

	BBox BBox::Union(const Vector3dF &p) {
		BBox ret = *this;
		ret.pMin.x = min(ret.pMin.x, p.x);
		ret.pMin.y = min(ret.pMin.y, p.y);
		ret.pMin.z = min(ret.pMin.z, p.z);
		ret.pMax.x = max(ret.pMax.x, p.x);
		ret.pMax.y = max(ret.pMax.y, p.y);
		ret.pMax.z = max(ret.pMax.z, p.z);
		return ret;
	}

	BBox Union(const BBox &b1, const BBox &b2) {
		BBox ret;
		ret.pMin.x = min(b1.pMin.x, b2.pMin.x);
		ret.pMin.y = min(b1.pMin.y, b2.pMin.y);
		ret.pMin.z = min(b1.pMin.z, b2.pMin.z);
		ret.pMax.x = max(b1.pMax.x, b2.pMax.x);
		ret.pMax.y = max(b1.pMax.y, b2.pMax.y);
		ret.pMax.z = max(b1.pMax.z, b2.pMax.z);
		return ret;
	}

	bool BBox::Overlaps(const BBox &b) const {
		bool overlap_x = (pMax.x >= b.pMin.x) && (pMin.x <= b.pMax.x);
		bool overlap_y = (pMax.y >= b.pMin.y) && (pMin.y <= b.pMax.y);
		bool overlap_z = (pMax.z >= b.pMin.z) && (pMin.z <= b.pMax.z);
		return (overlap_x && overlap_y && overlap_z);
	}

	bool BBox::Inside(const Vector3dF &p) const {
		return (p.x >= pMin.x && p.x <= pMax.x &&
				p.y >= pMin.y && p.y <= pMax.y &&
				p.z >= pMin.z && p.z <= pMax.z);
	}

	bool BBox::IsEmpty() const {
		if (pMin.x == INFINITY || pMin.x == INFINITY || pMin.x == INFINITY ||
			pMax.x == -INFINITY || pMax.x == -INFINITY || pMax.x == -INFINITY)
			return true;
		return false;
	}

	void BBox::Expand(float delta) {
		pMin -= Vector3dF(delta, delta, delta);
		pMax += Vector3dF(delta, delta, delta);
	}

	float BBox::SurfaceArea() const {
		Vector3dF d = pMax - pMin;
		return 2.f * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	float BBox::Volume() const {
		Vector3dF d = pMax - pMin;
		return d.x * d.y * d.z;
	}

	Axis BBox::MaximumExtent() const {
		Vector3dF diag = pMax - pMin;
		if (diag.x > diag.y && diag.x > diag.z)
			return Axis::x;
		else if (diag.y > diag.z)
			return Axis::y;
		else
			return Axis::z;
	}

	Vector3dF BBox::Lerp(float tx, float ty, float tz) const {
		return Vector3dF(renderer::Lerp(tx, pMin.x, pMax.x), renderer::Lerp(ty, pMin.y, pMax.y),
			renderer::Lerp(tz, pMin.z, pMax.z));
	}

	Vector3dF BBox::Offset(const Vector3dF &p) const {
		return Vector3dF((p.x - pMin.x) / (pMax.x - pMin.x),
			(p.y - pMin.y) / (pMax.y - pMin.y),
			(p.z - pMin.z) / (pMax.z - pMin.z));
	}

	void BBox::BoundingSphere(Vector3dF *c, float *rad) const {
		*c = .5f * pMin + .5f * pMax;
		*rad = Inside(*c) ? (*c - pMax).Length() : 0.f;
	}

	bool BBox::Intersect(const Ray &ray, float *hitt0, float *hitt1) const {
		float t0 = 0, t1 = ray.tMax;
		for (int i = 0; i < 3; ++i) { //x y z
			float invRayDir = 1.f / ray.d[i];
			float tNear = (pMin[i] - ray.o[i]) * invRayDir;
			float tFar = (pMax[i] - ray.o[i]) * invRayDir;
			if (tNear > tFar)
				std::swap(tNear, tFar);
			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1) {
				return false;
			}
		}
		if (hitt0) *hitt0 = t0;
		if (hitt1) *hitt1 = t1;
		return true;
	}

}