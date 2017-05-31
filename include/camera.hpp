#ifndef RENDERER_CAMERA_HPP
#define RENDERER_CAMERA_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "ray.hpp"
#include "matrix.hpp"

namespace renderer {
    class CameraBase {
    protected:
        Matrix4x4 cameraMat;
    protected:
        CameraBase() {}
        virtual void UpdateMatrix() {}
    public:
        Matrix4x4 GetMatrix() { return cameraMat; }
        virtual void SetCameraPosition(Vector3dF p) { }
        virtual Vector3dF GetCameraPosition() { return Vector3dF(0.0f, 0.0f, 0.0f); }
    };
    
    class PerspectiveCamera: public CameraBase {
        Matrix4x4 viewMat;
        Matrix4x4 projMat;
		Vector3dF eye;
		Vector3dF front;
		Vector3dF up;
		Vector3dF right;
        float fov;
        float near;
        float far;
        float aspect;
		float fovScale;
	public:
		PerspectiveCamera(
            const Vector3dF& eye = Vector3dF(0.0f, 0.0f, 0.0f),
            const Vector3dF& front = Vector3dF(0.0f, 0.0f, -1.0f),
            const Vector3dF& up = Vector3dF(0.0f, 1.0f, 0.0f),
            const float fov = 45.0f,
            const float ar = 1.0f,
            const float n = 0.1f,
            const float f = 10000.0f);
		PerspectiveCamera(const PerspectiveCamera& c) {
			*this = c;
		}
		PerspectiveCamera(PerspectiveCamera&& c) {
			*this = std::forward<PerspectiveCamera>(c);
		}
		PerspectiveCamera& operator = (PerspectiveCamera&& c) {
			eye = c.eye;
			front = c.front;
			up = c.up;
			fov = c.fov;
            aspect = c.aspect;
			fovScale = c.fovScale;
			right = c.right;
			return *this;
		}
		PerspectiveCamera& operator = (const PerspectiveCamera& c) {
			eye = c.eye;
			front = c.front;
			up = c.up;
			fov = c.fov;
            aspect = c.aspect;
			fovScale = c.fovScale;
			right = c.right;
			return *this;
        }
        void SetFov(float f) {
            fov = f;
            UpdateProjMatrix();
        }
        void SetAspect(float ar) {
            aspect = ar;
            UpdateProjMatrix();
        }
        void SetNear(float n) {
            near = n;
            UpdateProjMatrix();
        }
        void SetFar(float f) {
            far = f;
            UpdateProjMatrix();
        }
        void SetUpVector(Vector3dF v) {
            up = v;
            UpdateViewMatrix();
        }
        void SetFrontVector(Vector3dF v) {
            front = v;
            UpdateViewMatrix();
        }
        virtual void SetCameraPosition(Vector3dF p) {
            eye = p;
            UpdateViewMatrix();
        }
        virtual Vector3dF GetCameraPosition() {
            return eye;
        }
		void Init();
        Ray GenerateRay(float x, float y);
    protected:
        virtual void UpdateMatrix();
        void UpdateViewMatrix();
        void UpdateProjMatrix();
	};
}

#endif // RENDERER_CAMERA_HPP
