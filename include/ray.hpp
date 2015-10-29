#ifndef RENDERER_RAY_HPP
#define RENDERER_RAY_HPP

#include "base.hpp"
namespace renderer {
	class Ray {
	public:
		PtrVector origin;
		PtrVector direction;
	public:
		Ray(PtrVector origin, PtrVector direction);
		PtrVector GetPoint(float t);
	};
}
#endif // RENDERER_RAY_HPP
