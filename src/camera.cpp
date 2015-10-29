#include "camera.hpp"
#include <cmath>
#include <vector.hpp>
#include <ray.hpp>

namespace renderer {

	PerspectiveCamera::PerspectiveCamera(PtrVector eye, PtrVector front, PtrVector up, float fov)
		: eye(eye)
		, front(front)
		, refUp(up)
		, fov(fov)
	{
	}

	void PerspectiveCamera::Init() {
		right = std::make_shared<Vector>(front->Cross(*refUp));
		up = std::make_shared<Vector>(right->Cross(*front));
		fovScale = tan(fov * 0.5f * PI / 180.0f) * 2.0f;
	}

	PtrRay PerspectiveCamera::GenerateRay(float x, float y) {
		Vector&& r = (*right) * ((x - 0.5f) * fovScale);
		Vector&& u = (*up) * ((y - 0.5f) * fovScale);
		Vector&& v = (*front + r + u).Normalize();
		PtrRay ray = std::make_shared<Ray>(Ray(eye, std::make_shared<Vector>(v)));
		return ray;
	}

}