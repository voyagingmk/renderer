#include "stdafx.h"
#include "camera.hpp"
#include "geometry.hpp"
#include "ray.hpp"

namespace renderer {

	PerspectiveCamera::PerspectiveCamera(const Vector3dF& eye, const Vector3dF& front, const Vector3dF& up, const float fov, const float ar, const float n, const float f)
		: eye(eye)
		, front(front)
		, up(up)
        , fov(fov)
        , near(n)
        , far(f)
        , aspect(ar)
	{
	}
    
    void PerspectiveCamera::UpdateViewMatrix() {
        viewMat = LookAt(eye, front, up);
        cameraMat = viewMat * projMat;
    }
    
    void PerspectiveCamera::UpdateProjMatrix() {
        projMat = Perspective(fov, aspect, near, far);
        cameraMat = viewMat * projMat;
    }
    
    void PerspectiveCamera::UpdateMatrix() {
        viewMat = LookAt(eye, front, up);
        projMat = Perspective(fov, aspect, near, far);
        cameraMat = viewMat * projMat;
    }
    
	void PerspectiveCamera::Init() {
		right = front.Cross(up);
		up = right.Cross(front);
		fovScale = tan(0.5f * fov *  PI / 180.0f) * 2.0f;
        UpdateMatrix();
	}

	Ray PerspectiveCamera::GenerateRay(float x, float y) {
		Vector3dF&& r = right * ((x - 0.5f) * fovScale);
		Vector3dF&& u = up * ((y - 0.5f) * fovScale);
		Vector3dF&& v = (front + r + u).Normalize();
		return Ray(eye, v);
	}

}
