#ifndef RENDERER_SYSTEM_BATCH_HPP
#define RENDERER_SYSTEM_BATCH_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/miscEvent.hpp"
#include "com/miscCom.hpp"

using namespace ecs;

namespace renderer {

	class BatchSystem : public System<BatchSystem>, public Receiver<BatchSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const StaticBatchEvent& evt);

		void receive(const ComponentAddedEvent<DynamicObjTag>& evt);

		void receive(const ComponentRemovedEvent<DynamicObjTag> &evt);

	private:
		void updateStaticRenderQueue();

		void updateDynamicRenderQueue();
	};
};


#endif
