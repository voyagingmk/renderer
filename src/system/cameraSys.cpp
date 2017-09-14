#include "stdafx.h"
#include "system/cameraSys.hpp"
#include "com/keyState.hpp"
#include "com/geometry.hpp"

using namespace std;

namespace renderer {


	void CameraSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		evtMgr.on<CustomSDLMouseMotionEvent>(*this);
	}


	void CameraSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

		auto com = objMgr.getSingletonComponent<KeyState>();
		auto cameraView = objMgr.getSingletonComponent<PerspectiveCameraView>();
		Vector3dF p = { 0.0f, 0.0f, 0.0f };
		auto& keyState = com->state;
		if (keyState[SDLK_w] == SDL_PRESSED)
		{
			p = cameraView->GetFrontVector().Normalize();
		}
		if (keyState[SDLK_s] == SDL_PRESSED)
		{
			p = -cameraView->GetFrontVector().Normalize();
		}
		if (keyState[SDLK_a] == SDL_PRESSED)
		{
			p = -cameraView->GetRightVector().Normalize();
		}
		if (keyState[SDLK_d] == SDL_PRESSED)
		{
			p = cameraView->GetRightVector().Normalize();
		}
		if (keyState[SDLK_q] == SDL_PRESSED)
		{
			p = cameraView->GetUpVector().Normalize();
		}
		if (keyState[SDLK_e] == SDL_PRESSED)
		{
			p = -cameraView->GetUpVector().Normalize();
		}

		cameraView->SetTargetVector(cameraView->GetTargetVector() + p * 0.2f);
		cameraView->SetCameraPosition(cameraView->GetCameraPosition() + p * 0.2f);
		//camera.SetTargetVector(p + Vector3dF(0.0, 0.0, -1.0));
		// cameraView->eye.debug();

		if (keyState[SDLK_y] == SDL_PRESSED)
		{
			float f = cameraView->far;
			f += 0.1f;
			cameraView->far = f;
		}

		if (keyState[SDLK_u] == SDL_PRESSED)
		{
			float f = cameraView->far;
			f -= 0.1f;
			cameraView->far = f;
		}
	}


	void  CameraSystem::receive(const CustomSDLMouseMotionEvent &evt) {
		auto cameraView = evt.obj.objMgr().getSingletonComponent<PerspectiveCameraView>();
		auto e = evt.e;
		//printf("mMotion, t:%u, state:%u, (%d,%d), rel:(%d,%d)\n", e.type, e.state, e.x, e.y, e.xrel, e.yrel);
		float scale = 0.3f;
		Vector2dF cameraVec;
		cameraVec = { scale * e.xrel, -scale * e.yrel };
		cameraView->yaw += cameraVec.x;
		cameraView->pitch += cameraVec.y;
		if (cameraView->pitch > 89.0f)
			cameraView->pitch = 89.0f;
		if (cameraView->pitch < -89.0f)
			cameraView->pitch = -89.0f;
		Vector3dF front;
		front.x = cos(Radians(cameraView->yaw)) * cos(Radians(cameraView->pitch)); // 0
		front.y = sin(Radians(cameraView->pitch));                     // 0
		front.z = sin(Radians(cameraView->yaw)) * cos(Radians(cameraView->pitch)); // -1
		front = front.Normalize();
		cameraView->SetTargetVector(cameraView->GetCameraPosition() + front);
	}

};