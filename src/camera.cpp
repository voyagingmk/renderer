#include "stdafx.h"
#include "camera.hpp"
#include "geometry.hpp"
#include "ray.hpp"

namespace renderer {

	PerspectiveCamera::PerspectiveCamera(const Vector3dF& eye, const Vector3dF& front, const Vector3dF& up, const float fov)
		: eye(eye)
		, front(front)
		, refUp(up)
		, fov(fov)
	{
	}

	void PerspectiveCamera::Init() {
		right = front.Cross(refUp);
		up = right.Cross(front);
		fovScale = tan(fov * 0.5f * PI / 180.0f) * 2.0f;
	}

	Ray PerspectiveCamera::GenerateRay(float x, float y) {
		Vector3dF&& r = right * ((x - 0.5f) * fovScale);
		Vector3dF&& u = up * ((y - 0.5f) * fovScale);
		Vector3dF&& v = (front + r + u).Normalize();
		return Ray(eye, v);
	}

}
