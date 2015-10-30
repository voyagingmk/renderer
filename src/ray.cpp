#include "ray.hpp"
#include <geometry.hpp>

namespace renderer {


	Ray::Ray(Vector& origin, Vector& direction)
		: o(origin)
		, d(direction)
	{
	}

	Vector Ray::GetPoint(float t)
	{
		return o + d * t;
	}

}