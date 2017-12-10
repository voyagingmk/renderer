#ifndef RENDERER_SYSTEM_STAT_HPP
#define RENDERER_SYSTEM_STAT_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

using namespace ecs;

namespace renderer {
	class StatSystem : public System<StatSystem>, public Receiver<StatSystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
	};

};

#endif
