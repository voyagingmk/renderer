#include "stdafx.h"
#include "system/sceneGraphSys.hpp"
#include "utils/glutils.hpp"
#include "com/sdlContext.hpp"
#include "com/mesh.hpp"
#include "com/cameraCom.hpp"
#include "com/bufferCom.hpp"
#include "com/lightCom.hpp"
#include "com/bufferCom.hpp"
#include "com/miscCom.hpp"
#include "com/materialCom.hpp"
#include "com/shader.hpp"
#include "event/materialEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/spatialEvent.hpp"
#include "event/bufferEvent.hpp"
#include "event/textureEvent.hpp"
#include "event/lightEvent.hpp"

using namespace std;

namespace renderer {


	void SceneGraphSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("SceneGraphSystem init\n");
		evtMgr.on<RenderSceneEvent>(*this);
	}


	void SceneGraphSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void SceneGraphSystem::receive(const RenderSceneEvent &evt) {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		auto matSetCom = m_objMgr->getSingletonComponent<MaterialSet>();
		auto renderQueueCom = m_objMgr->getSingletonComponent<StaticRenderQueueCom>();
		glEnable(GL_DEPTH_TEST);
		setViewport(evt.viewport);
		clearView(evt.clearColor, evt.clearBits);
		Shader shader;

		if (evt.shader != nullptr) {
			shader = *evt.shader;
			shader.use();
		}
		MaterialSettingID preSettingID = -1;
		for (auto obj : m_objMgr->entities<MeshRef, StaticObjTag>()) {
			auto meshRef = obj.component<MeshRef>();
			m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
			m_evtMgr->emit<UploadMatrixToShaderEvent>(obj, shader);
			auto meshID = meshRef->meshID;
			Mesh& mesh = meshSet->meshDict[meshID];
			for (uint32_t subMeshIdx = 0; subMeshIdx < mesh.meshes.size(); subMeshIdx++) {
				SubMesh& subMesh = mesh.meshes[subMeshIdx];
				auto settingID = mesh.settingIDs[subMeshIdx];
				m_evtMgr->emit<ActiveMaterialEvent>(settingID, shader);
				m_evtMgr->emit<DrawMeshBufferEvent>(meshID, subMeshIdx);
			}
		}
		/*
		for (auto e: renderQueueCom->queue) {
		Object obj = m_objMgr->get(e.first);
		BufIdx bufIdx = e.second;
		auto matCom = obj.component<MaterialCom>();
		m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
		m_evtMgr->emit<UploadMatrixToShaderEvent>(obj, shader);
		auto meshBufferCom = obj.component<MeshBuffersCom>();
		auto meshBuffer = meshBufferCom->buffers[bufIdx];
		auto settingID = matCom->settingIDs[meshBuffer.matIdx];
		if (preSettingID != settingID) {
		auto setting = matSetCom->settingDict[settingID];
		if (evt.shader == nullptr) {
		shader = getShader(setting);
		shader.use();
		}
		m_evtMgr->emit<ActiveMaterialEvent>(settingID, shader);
		}
		m_evtMgr->emit<DrawSubMeshBufferEvent>(meshBuffer);
		preSettingID = settingID;
		// m_evtMgr->emit<DeactiveMaterialEvent>(settingID);
		}*/
	}

};