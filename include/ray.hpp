#ifndef RENDERER_RAY_HPP
#define RENDERER_RAY_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Ray {
	public:
		//float mint;
		//float maxt;
		Vector3dF o;//origin
		Vector3dF d;//direction
	public:
		Ray(Vector3dF& origin, Vector3dF& direction) noexcept;
		Vector3dF GetPoint(float t) noexcept;
	};
}
#endif // RENDERER_RAY_HPP
