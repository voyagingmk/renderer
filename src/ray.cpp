#include "ray.hpp"
#include <geometry.hpp>

namespace renderer {


	Ray::Ray(PtrVector origin, PtrVector direction)
		: origin(origin)
		, direction(direction)
	{
	}

	PtrVector Ray::GetPoint(float t)
	{
		PtrVector result = std::make_shared<Vector>((*origin) + (*direction) * t);
		return result;
	}

}