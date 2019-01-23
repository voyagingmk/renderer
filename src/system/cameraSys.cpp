#include "stdafx.h"
#include "system/cameraSys.hpp"
#include "com/keyState.hpp"
#include "com/geometry.hpp"
#include "event/miscEvent.hpp"

using namespace std;

namespace renderer {


	void CameraSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("CameraSystem init\n"); 
		evtMgr.on<CustomSDLMouseMotionEvent>(*this);
	}


	void CameraSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		auto com = objMgr.getSingletonComponent<KeyState>();
		for (const Object objCamera : m_objMgr->entities<PerspectiveCameraView, CameraRoamable>()) {
			auto cameraView = objCamera.component<PerspectiveCameraView>();
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

			cameraView->SetCameraPosition(cameraView->GetCameraPosition() + p * 1.0f);
			m_evtMgr->emit<CameraMoveEvent>(cameraView.object());
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
		
	}


	void  CameraSystem::receive(const CustomSDLMouseMotionEvent &evt) {
		for (const Object objCamera : m_objMgr->entities<PerspectiveCameraView, CameraRoamable>()) {
			auto cameraView = objCamera.component<PerspectiveCameraView>();
			auto e = evt.e;
			float scale = 0.3f;
			int xrel, yrel;
			SDL_GetRelativeMouseState(&xrel, &yrel);
			if (xrel == 0 && yrel == 0) {
				return;
			}
			Vector2dF vec;
			vec = { cameraView->mouseSense * xrel, cameraView->mouseSense * (-yrel) };
			cameraView->yaw += vec.x;
			cameraView->pitch += vec.y;
			if (cameraView->pitch > 89.0f)
				cameraView->pitch = 89.0f;
			if (cameraView->pitch < -89.0f)
				cameraView->pitch = -89.0f;
			UpdateCameraVectors(cameraView);
			// printf("mMotion, rel:(%d,%d), yaw: %.2f, pitch: %.2f\n", e.xrel, e.yrel, cameraView->yaw, cameraView->pitch);
		}
	}

	void  CameraSystem::UpdateCameraVectors(ComponentHandle<PerspectiveCameraView> cameraView) {
		Vector3dF front;
		front.x = cos(Radians(cameraView->yaw)) * cos(Radians(cameraView->pitch)); // 0
		front.y = sin(Radians(cameraView->pitch));                     // 0
		front.z = sin(Radians(cameraView->yaw)) * cos(Radians(cameraView->pitch)); // -1
		front = front.Normalize();
		cameraView->SetFrontVector(front);
	}

};
