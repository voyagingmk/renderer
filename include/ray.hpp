#ifndef RENDERER_RAY_HPP
#define RENDERER_RAY_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Ray {
	public:
		float tMax;
		Point3dF o;//origin
		Vector3dF d;//direction
	public:
		Ray(const Point3dF& origin, const Vector3dF& direction) noexcept;
		Ray(const Ray& r) noexcept;
		Point3dF GetPoint(float t) noexcept;
	};
}
#endif // RENDERER_RAY_HPP
