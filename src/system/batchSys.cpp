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
		// 关键点1：draw instance 必须是同材质，所以不同材质的obj不能放到instance里一起渲染
		// 关键点2：同Mesh的各个SubMesh可以共用instance buffer，因为各个SubMesh的model matrix一样
		// 关键点3：同MeshID的obj才可能组成instance，所以先根据MeshID分类
		// 关键点4：不同MeshID，但材质ID相同，是无法batch的

		// 步骤:
		// 1.根据MeshID对所有要渲染的obj做分类；
		// 2.遍历同MeshID的obj：
		//     遍历所有SubMesh，根据matID分类：
		//		  把objID放进 batch(MeshID, subMeshIdx, matID): objIDs 
		// 3.对于每一个batch(MeshID, matID)
		//     发射CreateInstanceBufferEvent
		//     生成一个objBatch，把batch(MeshID, subMeshIdx, matID): objIDs信息放进这个obj
		//     把instance buffer记录进这个obj
		
		// 渲染：
		// 1.遍历所有objBatch，读取batch(MeshID, subMeshIdx, matID)
		// 2.用meshIDD, subMeshIdx，找到SubMesh的MeshBufferRef
		// 3.根据matID设置材质
		// 4.发射BindInstanceBufferEvent(meshIDD, subMeshIdx, insBufferName)
		// 5.此时就准备好了instance draw的条件，差设置instance的model matrix
		// 6.遍历batched的objIDs，生成model matrix数组, 发射UpdateInstanceBufferEvent
		// 7.发射DrawSubMeshBufferEvent
		// 8.发射UnbindInstanceBufferEvent(meshIDD, subMeshIdx, insBufferName)

		// collect all ID
       //  objBatch-> [subMesh: MatID]: [objScene1, objScene2, ...]
        
        // [ [objScene, objMesh, subMeshBufferIdx] ]
		for (const Object objScene : m_objMgr->entities<MeshRef, RenderQueueTag, StaticObjTag>()) {
			auto meshRef = objScene.component<MeshRef>();
			Mesh& mesh = meshSet->getMesh(*meshRef.get());
            for (BufIdx subMeshIdx = 0; subMeshIdx < mesh.meshes.size(); subMeshIdx++) {
               // queue.push_back(std::make_pair(obj.ID(), idx));
            }
		}
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
