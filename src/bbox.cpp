#include "stdafx.h"
#include "bbox.hpp"

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

	BBox& BBox::operator = (const BBox& b) {
		pMin = b.pMin;
		pMax = b.pMax;
		return *this;
	};

	BBox& BBox::operator = (BBox&& b) {
		pMin = b.pMin;
		pMax = b.pMax;
		return *this;
	};

	BBox BBox::Union(const BBox &b, const Vector3dF &p) {
		BBox ret = b;
		ret.pMin.x = min(b.pMin.x, p.x);
		ret.pMin.y = min(b.pMin.y, p.y);
		ret.pMin.z = min(b.pMin.z, p.z);
		ret.pMax.x = max(b.pMax.x, p.x);
		ret.pMax.y = max(b.pMax.y, p.y);
		ret.pMax.z = max(b.pMax.z, p.z);
		return ret;
	}

	bool BBox::Overlaps(const BBox &b) const {
		bool x = (pMax.x >= b.pMin.x) && (pMin.x <= b.pMax.x);
		bool y = (pMax.y >= b.pMin.y) && (pMin.y <= b.pMax.y);
		bool z = (pMax.z >= b.pMin.z) && (pMin.z <= b.pMax.z);
		return (x && y && z);
	}

	bool BBox::Inside(const Vector3dF &p) const {
		return (p.x >= pMin.x && p.x <= pMax.x &&
			p.y >= pMin.y && p.y <= pMax.y &&
			p.z >= pMin.z && p.z <= pMax.z);
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

	int BBox::MaximumExtent() const {
		Vector3dF diag = pMax - pMin;
		if (diag.x > diag.y && diag.x > diag.z)
			return 0;
		else if (diag.y > diag.z)
			return 1;
		else
			return 2;
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

}