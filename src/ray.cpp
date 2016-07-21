#include "stdafx.h"
#include "ray.hpp"

namespace renderer {


	Ray::Ray(const Point3dF& origin, const Vector3dF& direction) noexcept
		: o(origin)
		, d(direction)
		, mint(0.f)
		, maxt(INFINITY)
	{
	}

	Ray::Ray(const Ray& r) noexcept
		:o(r.o)
		, d(r.d)
		, mint(r.mint)
		, maxt(r.maxt)
	{
	}

	Point3dF Ray::GetPoint(float t) noexcept
	{
		return o + d * t;
	}

}