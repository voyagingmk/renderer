#include "stdafx.h"
#include "system/batchSys.hpp"
#include "com/bufferCom.hpp"
#include "com/materialCom.hpp"
#include "com/mesh.hpp"

using json = nlohmann::json;
using namespace std;

namespace renderer {

	void BatchSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("BatchSystem init\n"); 

    }

	void BatchSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        
    }


	void BatchSystem::receive(const StaticBatchEvent& evt) {
		updateStaticRenderQueue();
	}

	void BatchSystem::receive(const ComponentAddedEvent<DynamicObjTag> &evt) {
		updateDynamicRenderQueue();
	}

	void BatchSystem::receive(const ComponentRemovedEvent<DynamicObjTag> &evt) {
		updateDynamicRenderQueue();
	}

	void BatchSystem::updateStaticRenderQueue() {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		auto queueCom = m_objMgr->getSingletonComponent<StaticRenderQueueCom>();
		RenderQueue& queue = queueCom->queue;
		queue.clear();
		std::map<MaterialSettingID,
			std::map<std::pair<MeshID, SubMeshIdx>, std::vector<ObjectID>>> batchInfo;
        auto objBatchList = m_objMgr->entities<StaticBatchObjTag>();
        for(auto objBatch: objBatchList) {
            objBatch.destroy();
        }
		// 所谓batch是指把同材质、同MeshID的obj一起渲染
		// 关键点1：draw instance 必须是同材质，所以不同材质的SubMesh不能放到instance里一起渲染
		// 关键点2：同Mesh的各个SubMesh可以共用instance buffer，因为各个SubMesh的model matrix一样
		// 关键点3：同MeshID的obj才可能组成instance，所以先根据MeshID分类
		// 关键点4：不同MeshID，但材质ID相同，是无法batch的
        // 假设MeshRef没有自定义SubMesh材质，那么同MeshID的objs，只需要创建一个instance buffer，
        // 有多少个SubMesh，就需要多少个drawCall，这些drawcall共用这个instance buffer
		// 步骤:
		// 1.遍历待渲染的obj：
		//     遍历所有obj的SubMesh，并求出matID：
		//		  把objID放进 batch(MeshID, subMeshIdx, matID): objIDs
		// 2.对于每一个batchKey
		//     发射CreateInstanceBufferEvent
		//     生成一个objBatch，把batch(MeshID, subMeshIdx, matID): objIDs信息放进这个obj
		//     把instance buffer记录进这个obj
        
        std::map<MeshID, std::vector<ObjectID>> meshID2ObjIDs;
        std::map<std::tuple<MeshID, SubMeshIdx, MaterialSettingID>, std::vector<ObjectID>> batchKey2ObjIDs;
        for (const Object objScene : m_objMgr->entities<MeshRef, RenderableTag, StaticObjTag>()) {
            auto meshRef = objScene.component<MeshRef>();
            MeshID meshID = meshRef->meshID;
            Mesh& mesh = meshSet->getMesh(meshID);
            for (SubMeshIdx idx = 0; idx < mesh.meshes.size(); idx++) {
                MaterialSettingID settingID = mesh.settingIDs[idx];
                if (meshRef->customSettingIDDict[idx]) {
                    settingID = meshRef->customSettingIDDict[idx];
                }
                // meshID2ObjIDs
                auto key = std::make_tuple(meshID, idx, settingID);
                if (batchKey2ObjIDs.find(key) == batchKey2ObjIDs.end()) {
                    batchKey2ObjIDs.insert({key, {}});
                }
                std::vector<ObjectID>& objIDs = batchKey2ObjIDs[key];
                objIDs.push_back(objScene.ID());
            }
        }
        for (auto it: batchKey2ObjIDs) {
            auto key = it.first;
            auto objIDs = it.second;
            auto objBatch = m_objMgr->create();
            auto batchInfoCom = objBatch.addComponent<BatchInfoCom>();
            MeshID meshID;
            SubMeshIdx subMeshIdx;
            MaterialSettingID settingID;
            std::tie(meshID, subMeshIdx, settingID) = key;
            batchInfoCom->meshID = meshID;
            batchInfoCom->settingID = settingID;
            batchInfoCom->subMeshIdx = subMeshIdx;
            batchInfoCom->objIDs = objIDs;
            batchInfoCom->modelMatrixes.resize(objIDs.size());
            // 把obj的mdel矩阵填进去
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
