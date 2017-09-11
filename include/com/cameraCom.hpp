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
			right( 1.0f, 0.0f, 0.0f )
		{}
		Vector3dF eye;
		Vector3dF target;
		Vector3dF up;
		Vector3dF right;
		Matrix4x4 cameraMat;
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
