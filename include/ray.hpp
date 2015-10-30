#ifndef RENDERER_RAY_HPP
#define RENDERER_RAY_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Ray {
	public:
		Vector o;//origin
		Vector d;//direction
	public:
		Ray(Vector& origin, Vector& direction);
		Vector GetPoint(float t);
	};
}
#endif // RENDERER_RAY_HPP
