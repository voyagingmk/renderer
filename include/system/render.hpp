#ifndef RENDERER_SYSTEM_RENDER_HPP
#define RENDERER_SYSTEM_RENDER_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"



using namespace ecs;

namespace renderer {
	class RenderSystem : public System<RenderSystem>, public Receiver<RenderSystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr);
		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt);
	};

};

#endif
