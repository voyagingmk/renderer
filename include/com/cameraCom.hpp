#ifndef RENDERER_COM_CAMERA_HPP
#define RENDERER_COM_CAMERA_HPP

#include "base.hpp"
#include "com/geometry.hpp"
#include "com/matrix.hpp"


namespace renderer {

	class CameraView {
	public:
		CameraView() :
			eye( 0.0f, 0.0f, 1.0f ),
			target( 0.0f, 0.0f, 0.0f ),
			up( 0.0f, 1.0f, 0.0f ),
			right( 1.0f, 0.0f, 0.0f ),
			yaw(-180.0f),
			pitch(0)
		{}
		Vector3dF eye;
		Vector3dF target;
		Vector3dF up;
		Vector3dF right;
		Matrix4x4 cameraMat;
		float yaw;
		float pitch;
		
		Vector3dF GetCameraPosition() {
			return eye;
		}

		Vector3dF GetTargetVector() {
			return target;
		}

		Vector3dF GetFrontVector() {
			return target - eye;
		}

		Vector3dF GetUpVector() {
			return up;
		}

		Vector3dF GetRightVector() {
			return right;
		}

		void SetTargetVector(Vector3dF v) {
			target = v;
			UpdateRightVector();
		}

		void SetCameraPosition(Vector3dF p) {
			eye = p;
			UpdateRightVector();
		}

		void UpdateRightVector() {
			Vector3dF focal = (target - eye).Normalize();
			right = (focal.Cross(up)).Normalize();
		}
	};

	class PerspectiveCameraView: public CameraView {
	public:
		PerspectiveCameraView() :
			fov(45.0f),
			aspect(1.0f),
			near(0.1f),
			far(1000.0f),
			fovScale(1.0f)
		{}
		float fov;
		float aspect;
		float near;
		float far;
		float fovScale;

		float GetFov() {
			return fov;
		}

		float GetNear() {
			return near;
		}

		float GetFar() {
			return far;
		}
	};


	class OrthoCameraView : public CameraView {
	public:
		float left;
		float right;
		float bottom;
		float top;
		float zNear;
		float zFar;
	};
};

#endif
