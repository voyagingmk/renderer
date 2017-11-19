#include "stdafx.h"
#include "system/lightSys.hpp"
#include "com/spatialData.hpp"
#include "event/bufferEvent.hpp"
#include "com/bufferCom.hpp"

using namespace std;

namespace renderer {
    void LightSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("LightSystem init\n");
        evtMgr.on<UpdateLightEvent>(*this);
        evtMgr.on<AddLightEvent>(*this);
		evtMgr.on<UpdateSpatialDataEvent>(*this);
		evtMgr.on<EnableLightShadowEvent>(*this);
		evtMgr.on<DisableLightShadowEvent>(*this);
    }
    
    void LightSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        
    }

    void LightSystem::receive(const UpdateLightEvent &evt) {
		updateLight(evt.obj);
    }

    void LightSystem::receive(const AddLightEvent &evt) {  
		updateLight(evt.obj);
    }

	void LightSystem::receive(const UpdateSpatialDataEvent &evt) {
		updateLight(evt.obj);
	}

	void LightSystem::updateLight(Object obj) {
		if (obj.hasComponent<PointLightTransform>()) {
			updatePointLight(obj);
		}
		else if (obj.hasComponent<DirLightTransform>()) {
			updateDirLight(obj);
		}
	}

	void LightSystem::updatePointLight(Object obj) {
		auto spatialData = obj.component<SpatialData>();
		auto com = obj.component<PointLightTransform>();
		Matrix4x4 shadowProj;
		shadowProj = Perspective(90.0f, com->aspect, com->n, com->f);
		Vector3dF lightPos = spatialData->pos;
		if (com->lightPVs.size() == 0) {
			com->lightPVs.resize(6);
		}
		com->lightPVs[0] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 });  // right
		com->lightPVs[1] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }); // left
		com->lightPVs[2] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 });   // top
		com->lightPVs[3] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, -1.0, 0.0 }, { 0.0, 0.0, -1.0 }); // bottom
		com->lightPVs[4] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, 0.0, 1.0 }, { 0.0, -1.0, 0.0 });  // near
		com->lightPVs[5] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, 0.0, -1.0 }, { 0.0, -1.0, 0.0 }); // far
	}

	void LightSystem::updateDirLight(Object obj) {
		auto spatialData = obj.component<SpatialData>();
		auto com = obj.component<DirLightTransform>();
		Matrix4x4 shadowProj = Ortho(-com->size, com->size, -com->size, com->size, com->n, com->f);
		Vector3dF lightPos = spatialData->pos;
		com->lightPV = shadowProj * LookAt(lightPos, Vector3dF{ 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 });
	}


	void LightSystem::receive(const EnableLightShadowEvent &evt) {
		auto aliasname = "lightDepth" + std::to_string(evt.obj.ID());
		if (evt.obj.hasComponent<PointLightCom>()) {
			m_evtMgr->emit<CreateDpethBufferEvent>(
				aliasname.c_str(),
				aliasname.c_str(),
				DepthTexType::CubeMap,
				1024);
		}
		else if (evt.obj.hasComponent<DirLightCom>()) {
			m_evtMgr->emit<CreateDpethBufferEvent>(
				aliasname.c_str(),
				aliasname.c_str(),
				DepthTexType::DepthOnly,
				1024);
		}
	}

	void LightSystem::receive(const DisableLightShadowEvent &evt) {
		auto aliasname = "lightDepth" + std::to_string(evt.obj.ID());
		m_evtMgr->emit<DestroyColorBufferEvent>(aliasname.c_str());
	}
};