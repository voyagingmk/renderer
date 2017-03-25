#include "stdafx.h"
#include "ray.hpp"

namespace renderer {


	Ray::Ray(const Point3dF& origin, const Vector3dF& direction) noexcept
		: o(origin)
		, d(direction)
		, tMax(INFINITY)
	{
	}

	Ray::Ray(const Ray& r) noexcept
		:o(r.o)
		, d(r.d)
		, tMax(r.tMax)
	{
	}

	Point3dF Ray::GetPoint(float t) noexcept
	{
		return o + d * t;
	}

}