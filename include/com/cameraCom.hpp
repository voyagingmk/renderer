#ifndef RENDERER_COM_CAMERA_HPP
#define RENDERER_COM_CAMERA_HPP

#include "base.hpp"
#include "com/geometry.hpp"
#include "com/matrix.hpp"


namespace renderer {

	class CameraView {
	public:
		CameraView() :
			eye( 0.0f, 0.0f, 0.0f ),
			front( 0.0f, 0.0f, -1.0f ),
			up( 0.0f, 1.0f, 0.0f ),
			right( 1.0f, 0.0f, 0.0f ),
			yaw(-90.0f),
			pitch(0.0f),
			mouseSense(0.1f),
			moveSpeed(2.5f),
			zoom(45.0f)
		{}
		Vector3dF eye;
		Vector3dF front;
		Vector3dF up;
		Vector3dF right;
		Matrix4x4 cameraMat;
		float yaw;
		float pitch;
		float moveSpeed;
		float mouseSense;
		float zoom;
		
		Vector3dF GetCameraPosition() {
			return eye;
		}

		Vector3dF GetTargetVector() {
			return eye + front;
		}

		Vector3dF GetFrontVector() {
			return front;
		}

		Vector3dF GetUpVector() {
			return up;
		}

		Vector3dF GetRightVector() {
			return right;
		}

		void SetFrontVector(Vector3dF v) {
			front = v;
			UpdateCameraVector();
		}

		void SetCameraPosition(Vector3dF p) {
			eye = p;
			UpdateCameraVector();
		}

		void UpdateCameraVector() {
			const Vector3dF worldUp(0.0f, 1.0f, 0.0f);
			right = (front.Cross(worldUp)).Normalize();
			up = (right.Cross(front)).Normalize();
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
