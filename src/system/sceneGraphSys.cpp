#include "stdafx.h"
#include "system/sceneGraphSys.hpp"
#include "utils/glutils.hpp"
#include "com/sdlContext.hpp"
#include "com/mesh.hpp"
#include "com/cameraCom.hpp"
#include "com/spatialData.hpp"
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
		glEnable(GL_DEPTH_TEST);
		setViewport(evt.viewport);
		clearView(evt.clearColor, evt.clearBits);
		Shader shader;

		if (evt.shader != nullptr) {
			shader = *evt.shader;
			shader.use();
		}

		m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);

		for (auto obj : m_objMgr->entities<RootNodeTag>()) {
			RenderNode(obj, shader);
		}
		/*
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
		}*/
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

	void SceneGraphSystem::RenderNode(Object obj, Shader shader) {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		auto matSetCom = m_objMgr->getSingletonComponent<MaterialSet>();
		
		if (obj.hasComponent<BatchObjectListCom>()) {
			auto com = obj.component<BatchObjectListCom>();
			shader.set1b("instanced", true);
			auto objIDs = com->objIDs;
			DrawBatchObjs(shader, objIDs);
			/* z pre-pass way
			glDepthFunc(GL_LESS);
			glColorMask(0, 0, 0, 0);
			DrawBatchObjs(shader, objIDs);
			glDepthFunc(GL_LEQUAL);
			glColorMask(1, 1, 1, 1);
			glDepthMask(GL_FALSE);
			DrawBatchObjs(shader, objIDs);
			glDepthMask(GL_TRUE);
			*/
			shader.set1b("instanced", false);
		}

		if (obj.hasComponent<RenderableTag>() && obj.hasComponent<DynamicObjTag>()) {
			auto meshRef = obj.component<MeshRef>();
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

		auto sgNode = obj.component<SceneGraphNode>();
		for (auto childObjID : sgNode->children) {
			Object childObj = m_objMgr->get(childObjID);
			RenderNode(childObj, shader);
		}
	}

	void SceneGraphSystem::DrawBatchObjs(Shader shader, std::vector<ecs::ObjectID>& objIDs) {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		for (auto objID : objIDs) {
			Object objBatch = m_objMgr->get(objID);
			auto batchInfoCom = objBatch.component<BatchInfoCom>();
			Mesh& mesh = meshSet->meshDict[batchInfoCom->meshID];
			for (uint32_t subMeshIdx = 0; subMeshIdx < mesh.meshes.size(); subMeshIdx++) {
				auto settingID = mesh.settingIDs[subMeshIdx];
				m_evtMgr->emit<BindInstanceBufferEvent>(
					batchInfoCom->meshID,
					subMeshIdx,
					batchInfoCom->bufferName);
				m_evtMgr->emit<ActiveMaterialEvent>(settingID, shader);
				m_evtMgr->emit<DrawMeshBufferEvent>(batchInfoCom->meshID, subMeshIdx);
				m_evtMgr->emit<UnbindInstanceBufferEvent>(
					batchInfoCom->meshID,
					subMeshIdx);
			}
		}
	}
};