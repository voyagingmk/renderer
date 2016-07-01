#ifndef BBOX_HPP
#define BBOX_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "ray.hpp"

namespace renderer {
	class BBox {
	public:
		Vector3dF pMin = { INFINITY, INFINITY, INFINITY };
		Vector3dF pMax = { -INFINITY, -INFINITY, -INFINITY };
	public:
		BBox() noexcept;
		BBox(const Vector3dF &p) noexcept;
		BBox(const Vector3dF& p1, const Vector3dF& p2) noexcept;
		BBox(const BBox& b) noexcept;
		BBox(BBox&& b) noexcept;
		~BBox();
		BBox& operator = (const BBox& b) noexcept;
		BBox& operator = (BBox&& b) noexcept;
		BBox Union(const BBox &b, const Vector3dF &p);
		friend BBox Union(const BBox &b1, const BBox &b2);
		bool Overlaps(const BBox &b) const;
		bool Inside(const Vector3dF &pt) const;
		void Expand(float delta);
		float SurfaceArea() const;
		float Volume() const;
		int MaximumExtent() const;
		Vector3dF Lerp(float tx, float ty, float tz) const;
		Vector3dF Offset(const Vector3dF &p) const; 
		void BoundingSphere(Vector3dF *c, float *rad) const;
		inline const Vector3dF &operator[](int i) const {
			Assert(i == 0 || i == 1);
			return (&pMin)[i];
		}
		inline Vector3dF &operator[](int i) {
			Assert(i == 0 || i == 1);
			return (&pMin)[i];
		}
		bool Intersect(const Ray &ray, float *hitt0, float *hitt1) const;
	};
}
#endif // BBOX_HPP
