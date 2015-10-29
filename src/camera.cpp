#include "camera.hpp"
#include <cmath>
#include <vector.hpp>
#include <ray.hpp>

namespace renderer {

	PerspectiveCamera::PerspectiveCamera(PtrVector eye, PtrVector front, PtrVector up, float fov)
		: m_eye(eye)
		, m_front(front)
		, m_refUp(up)
		, m_fov(fov)
	{
	}

	void PerspectiveCamera::init() {
		m_right = std::make_shared<Vector>(m_front->cross(*m_refUp));
		m_up = std::make_shared<Vector>(m_right->cross(*m_front));
		m_fovScale = tan(m_fov * 0.5f * PI / 180.0f) * 2.0f;
	}

	PtrRay PerspectiveCamera::generateRay(float x, float y) {
		Vector&& r = (*m_right) * ((x - 0.5f) * m_fovScale);
		Vector&& u = (*m_up) * ((y - 0.5f) * m_fovScale);
		Vector&& v = (*m_front + r + u).normalize();
		PtrRay ray = std::make_shared<Ray>(Ray(m_eye, std::make_shared<Vector>(v)));
		return ray;
	}

}