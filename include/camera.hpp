#ifndef RENDERER_CAMERA_HPP
#define RENDERER_CAMERA_HPP

#include "base.hpp"
namespace renderer {
	class PerspectiveCamera {
		PtrVector m_eye;
		PtrVector m_front;
		PtrVector m_up;
		PtrVector m_refUp;
		PtrVector m_right;
		float m_fov;
		float m_fovScale;
	public:
		PerspectiveCamera(PtrVector eye, PtrVector front, PtrVector up, float fov);
		void init();
		PtrRay generateRay(float x, float y);
	};
}

#endif // RENDERER_CAMERA_HPP
