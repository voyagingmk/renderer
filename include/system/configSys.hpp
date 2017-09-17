#ifndef RENDERER_SYSTEM_CONFIG_HPP
#define RENDERER_SYSTEM_CONFIG_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/miscEvent.hpp"

using namespace ecs;

namespace renderer {

	class ConfigSystem : public System<ConfigSystem>, public Receiver<ConfigSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const LoadConfigEvent& evt);
	};
};


#endif

