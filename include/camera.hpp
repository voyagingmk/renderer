#ifndef RENDERER_CAMERA_HPP
#define RENDERER_CAMERA_HPP

#include "base.hpp"
namespace renderer {
	class PerspectiveCamera {
		PtrVector eye;
		PtrVector front;
		PtrVector up;
		PtrVector refUp;
		PtrVector right;
		float fov;
		float fovScale;
	public:
		PerspectiveCamera(PtrVector eye, PtrVector front, PtrVector up, float fov);
		void Init();
		PtrRay GenerateRay(float x, float y);
	};
}

#endif // RENDERER_CAMERA_HPP
