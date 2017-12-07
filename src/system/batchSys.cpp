#include "stdafx.h"
#include "system/batchSys.hpp"
#include "com/bufferCom.hpp"
#include "com/materialCom.hpp"

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
		auto queueCom = m_objMgr->getSingletonComponent<StaticRenderQueueCom>();
		RenderQueue& queue = queueCom->queue;
		queue.clear();
		// collect all ID
		for (const Object obj : m_objMgr->entities<RenderQueueTag>()) {
			auto meshBufferCom = obj.component<MeshBuffersCom>();
			for (BufIdx idx = 0; idx < meshBufferCom->buffers.size(); idx++) {
				queue.push_back(std::make_pair(obj.ID(), idx));
			}
		}
		// sort by materialID
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
		});
	}

	void BatchSystem::updateDynamicRenderQueue() {

	}
};