#ifndef RENDERER_SYSTEM_RENDER_HPP
#define RENDERER_SYSTEM_RENDER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/renderEvent.hpp"
#include "com/materialCom.hpp"
#include "com/shader.hpp"



using namespace ecs;

namespace renderer {
	class RenderSystem : public System<RenderSystem>, public Receiver<RenderSystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const RenderSceneEvent &evt);

		void renderQuad();

		Shader getShader(MaterialSettingCom& matCom);

		Shader getShader(std::string shaderName);
	};

};

#endif
