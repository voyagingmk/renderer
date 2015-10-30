#include "stdafx.h"
#include "camera.hpp"
#include <cmath>
#include <geometry.hpp>
#include <ray.hpp>

namespace renderer {

	PerspectiveCamera::PerspectiveCamera(Vector& eye, Vector& front, Vector& up, float fov)
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
		Vector&& r = right * ((x - 0.5f) * fovScale);
		Vector&& u = up * ((y - 0.5f) * fovScale);
		Vector&& v = (front + r + u).Normalize();
		return Ray(eye, v);
	}

}