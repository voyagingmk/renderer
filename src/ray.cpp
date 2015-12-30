#include "stdafx.h"
#include "ray.hpp"
#include <geometry.hpp>

namespace renderer {


	Ray::Ray(Vector3dF& origin, Vector3dF& direction) noexcept
		: o(origin)
		, d(direction)
	{
	}

	Vector3dF Ray::GetPoint(float t) noexcept
	{
		return o + d * t;
	}

}