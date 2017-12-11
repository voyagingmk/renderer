#ifndef RENDERER_SYSTEM_SCENEGRAPH_HPP
#define RENDERER_SYSTEM_SCENEGRAPH_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/renderEvent.hpp"

using namespace ecs;

namespace renderer {
	class SceneGraphSystem : public System<SceneGraphSystem>, public Receiver<SceneGraphSystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const RenderSceneEvent &evt);

	private:

		void RenderNode(Object obj, Shader shader);

	};

};

#endif
