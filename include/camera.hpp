#ifndef RENDERER_CAMERA_HPP
#define RENDERER_CAMERA_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class PerspectiveCamera {
		Vector3dF eye;
		Vector3dF front;
		Vector3dF up;
		Vector3dF refUp;
		Vector3dF right;
		float fov;
		float fovScale;
	public:
		PerspectiveCamera(Vector3dF& eye, Vector3dF& front, Vector3dF& up, float fov);
		void Init();
		Ray GenerateRay(float x, float y);
	};
}

#endif // RENDERER_CAMERA_HPP
