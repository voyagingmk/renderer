#include "stdafx.h"
#include "ray.hpp"
#include <geometry.hpp>

namespace renderer {


	Ray::Ray(Vector& origin, Vector& direction) noexcept
		: o(origin)
		, d(direction)
	{
	}

	Vector Ray::GetPoint(float t) noexcept
	{
		return o + d * t;
	}

}