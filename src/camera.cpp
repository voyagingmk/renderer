#include "stdafx.h"
#include "camera.hpp"
#include "com/geometry.hpp"
#include "ray.hpp"

namespace renderer {

	PerspectiveCamera::PerspectiveCamera(const Vector3dF& eye, const Vector3dF& target, const Vector3dF& up, const float fov, const float ar, const float n, const float f)
		: eye(eye)
		, target(target)
		, up(up)
        , fov(fov)
        , aspect(ar)
        , near(n)
        , far(f)
	{
        Init();
	}
    
    void PerspectiveCamera::UpdateViewMatrix() {
        viewMat = LookAt(eye, target, up);
        cameraMat = projMat * viewMat;
    }
    
    void PerspectiveCamera::UpdateProjMatrix() {
        projMat = Perspective(fov, aspect, near, far);
        cameraMat = projMat * viewMat;
    }
    
    void PerspectiveCamera::UpdateMatrix() {
        viewMat = LookAt(eye, target, up);
        projMat = Perspective(fov, aspect, near, far);
        cameraMat = projMat * viewMat;
    }
    
	void PerspectiveCamera::Init() {
        /*
		right = focal.Cross(up);
		up = right.Cross(focal);
		fovScale = tan(0.5f * fov *  PI / 180.0f) * 2.0f;
        */
        UpdateMatrix();
	}

	Ray PerspectiveCamera::GenerateRay(float x, float y) {
		Vector3dF&& r = right * ((x - 0.5f) * fovScale);
		Vector3dF&& u = up * ((y - 0.5f) * fovScale);
        Vector3dF focal = (target - eye).Normalize();
		Vector3dF&& v = (focal + r + u).Normalize();
		return Ray(eye, v);
	}

}
