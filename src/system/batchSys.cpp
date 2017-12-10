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
		// ��νbatch��ָ��ͬ���ʡ�ͬMeshID��objһ����Ⱦ
		// �ؼ���1��draw instance ������ͬ���ʣ����Բ�ͬ���ʵ�obj���ܷŵ�instance��һ����Ⱦ
		// �ؼ���2��ͬMesh�ĸ���SubMesh���Թ���instance buffer����Ϊ����SubMesh��model matrixһ��
		// �ؼ���3��ͬMeshID��obj�ſ������instance�������ȸ���MeshID����
		// �ؼ���4����ͬMeshID��������ID��ͬ�����޷�batch��

		// ����:
		// 1.����MeshID������Ҫ��Ⱦ��obj�����ࣻ
		// 2.����ͬMeshID��obj��
		//     ��������SubMesh������matID���ࣺ
		//		  ��objID�Ž� batch(MeshID, subMeshIdx, matID): objIDs 
		// 3.����ÿһ��batch(MeshID, matID)
		//     ����CreateInstanceBufferEvent
		//     ����һ��objBatch����batch(MeshID, subMeshIdx, matID): objIDs��Ϣ�Ž����obj
		//     ��instance buffer��¼�����obj
		
		// ��Ⱦ��
		// 1.��������objBatch����ȡbatch(MeshID, subMeshIdx, matID)
		// 2.��meshIDD, subMeshIdx���ҵ�SubMesh��MeshBufferRef
		// 3.����matID���ò���
		// 4.����BindInstanceBufferEvent(meshIDD, subMeshIdx, insBufferName)
		// 5.��ʱ��׼������instance draw��������������instance��model matrix
		// 6.����batched��objIDs������model matrix����, ����UpdateInstanceBufferEvent
		// 7.����DrawSubMeshBufferEvent
		// 8.����UnbindInstanceBufferEvent(meshIDD, subMeshIdx, insBufferName)

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
