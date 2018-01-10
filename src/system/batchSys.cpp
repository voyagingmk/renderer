#include "stdafx.h"
#include "system/batchSys.hpp"
#include "com/bufferCom.hpp"
#include "com/materialCom.hpp"
#include "com/mesh.hpp"
#include "com/spatialData.hpp"
#include "event/bufferEvent.hpp"

using json = nlohmann::json;
using namespace std;

// 所谓batch是指把同材质、同MeshID的obj一起渲染
// 关键点1：draw instance 必须是同材质，所以不同材质的SubMesh不能放到instance里一起渲染
// 关键点2：同Mesh的各个SubMesh可以共用instance buffer，因为各个SubMesh的model matrix一样
// 关键点3：同MeshID的obj才可能组成instance，所以先根据MeshID分类
// 关键点4：不同MeshID，但材质ID相同，是无法batch的

namespace renderer {

	void BatchSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("BatchSystem init\n"); 
		evtMgr.on<UpdateBatchEvent>(*this);

    }

	void BatchSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        
    }


	void BatchSystem::receive(const UpdateBatchEvent& evt) {
		UpdateBatch(evt.obj, evt.recursive);
	}

	void BatchSystem::receive(const ComponentAddedEvent<DynamicObjTag> &evt) {
		updateDynamicRenderQueue();
	}

	void BatchSystem::receive(const ComponentRemovedEvent<DynamicObjTag> &evt) {
		updateDynamicRenderQueue();
	}

	// 对该场景节点的所有children做batch
	// batch信息记录在当前节点上
	// TODO 支持层级合并
	// TODO 支持动态batch
	// TODO 支持修改材质
	void BatchSystem::UpdateBatch(Object objScene, bool recursive) {
        // 假设MeshRef没有自定义SubMesh材质，那么同MeshID的objs，只需要创建一个instance buffer，
        // 有多少个SubMesh，就需要多少个drawCall，这些drawcall共用这个instance buffer
		// 步骤:
		// 1.遍历待渲染的obj：
		//		  把objID放进 batch(MeshID): objIDs
		// 2.对于每一个batchKey
		//     生成一个objBatch，把objIDs信息放进这个obj
		//     发射CreateInstanceBufferEvent
		//     把instance buffer name记录进这个obj

		auto sgNode = objScene.component<SceneGraphNode>();

        std::map<MeshID, std::vector<ObjectID>> meshID2ObjIDs;

		for (auto childObjID : sgNode->children) {
			Object childObj = m_objMgr->get(childObjID);
			if (!childObj.hasComponent<StaticObjTag>()) {
				continue;
			}
			if (!childObj.hasComponent<RenderableTag>()) {
				continue;
			}
			auto meshRef = childObj.component<MeshRef>();
			MeshID meshID = meshRef->meshID;
			if (meshID2ObjIDs.find(meshID) == meshID2ObjIDs.end()) {
				meshID2ObjIDs.insert({ meshID,{} });
			}
			std::vector<ObjectID>& objIDs = meshID2ObjIDs[meshID];
			objIDs.push_back(childObj.ID());
		}
		if (meshID2ObjIDs.size() > 0) {
			auto batchObjectList = objScene.addComponent<BatchObjectListCom>();
			// TODO 销毁旧的objBatch
			for (auto it : meshID2ObjIDs) {
				auto objIDs = it.second;
				auto objBatch = m_objMgr->create();
				auto batchInfoCom = objBatch.addComponent<BatchInfoCom>();
				batchInfoCom->meshID = it.first;
				batchInfoCom->objIDs = objIDs;
				batchInfoCom->modelMatrixes.resize(objIDs.size());
				batchInfoCom->bufferName = "batchNode" + std::to_string(objBatch.ID());
				// 把obj的mdel矩阵填进去
				//TODO 单独做一个update事件
				for (int i = 0; i < objIDs.size(); i++) {
					auto objID = objIDs[i];
					auto obj = m_objMgr->get(objID);
					auto spatialData = obj.component<SpatialData>();
					batchInfoCom->modelMatrixes[i] = spatialData->o2w.GetMatrix().transpose().dataRef();
				}
				auto matList = batchInfoCom->modelMatrixes;
				batchObjectList->objIDs.push_back(objBatch.ID());
				m_evtMgr->emit<CreateInstanceBufferEvent>(batchInfoCom->bufferName);
				m_evtMgr->emit<UpdateInstanceBufferEvent>(batchInfoCom->bufferName,
					batchInfoCom->modelMatrixes.size(),
					sizeof(Matrix4x4Value),
					&batchInfoCom->modelMatrixes[0],
                    false);
			}
		}

		if (recursive) {
			for (auto childObjID : sgNode->children) {
				Object childObj = m_objMgr->get(childObjID);
				UpdateBatch(childObj, recursive);
			}
		}
		// 渲染：
		// 1.遍历所有objBatch，读取batch(MeshID, subMeshIdx, matID)
		// 2.用meshIDD, subMeshIdx，找到SubMesh的MeshBufferRef
		// 3.根据matID设置材质
		// 4.发射BindInstanceBufferEvent(meshIDD, subMeshIdx, insBufferName)
		// 5.此时就准备好了instance draw的条件，差设置instance的model matrix
		// 6.遍历batched的objIDs，生成model matrix数组, 发射UpdateInstanceBufferEvent
		// 7.发射DrawSubMeshBufferEvent
		// 8.发射UnbindInstanceBufferEvent(meshIDD, subMeshIdx, insBufferName)

		// sort by materialID
		/*
		std::sort(queue.begin(), queue.end(), [&](std::pair<ObjectID, BufIdx> a, std::pair<ObjectID, BufIdx> b) -> bool {
			Object aObj = Object(m_objMgr, a.first);
			Object bObj = Object(m_objMgr, b.first);
			auto aCom = aObj.component<MeshBuffersCom>();
			auto aMatCom = aObj.component<MaterialCom>();
			auto bCom = bObj.component<MeshBuffersCom>();
			auto bMatCom = bObj.component<MaterialCom>();
			auto aBuf = aCom->buffers[a.second];
			auto bBuf = bCom->buffers[b.second];
			auto aSettingID = aMatCom->settingIDs[aBuf.matIdx];
			auto bSettingID = bMatCom->settingIDs[bBuf.matIdx];
			return aSettingID < bSettingID;
		});*/
	}

	void BatchSystem::updateDynamicRenderQueue() {

	}
};
