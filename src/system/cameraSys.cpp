#include "stdafx.h"
#include "com/cameraCom.hpp"
#include "system/cameraSys.hpp"

using namespace std;

namespace renderer {


	void CameraSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		Object camObj = objMgr.create();
		auto com = camObj.addComponent<PerspectiveCameraView>();
	}

	void CameraSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void receive(const WinResizeEvent &evt) {

	}

};