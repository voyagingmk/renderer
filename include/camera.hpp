#ifndef RENDERER_CAMERA_HPP
#define RENDERER_CAMERA_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class PerspectiveCamera {
		Vector eye;
		Vector front;
		Vector up;
		Vector refUp;
		Vector right;
		float fov;
		float fovScale;
	public:
		PerspectiveCamera(Vector& eye, Vector& front, Vector& up, float fov);
		void Init();
		Ray GenerateRay(float x, float y);
	};
}

#endif // RENDERER_CAMERA_HPP
