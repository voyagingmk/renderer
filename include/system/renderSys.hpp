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
        
    private:

        void setViewport(const Viewport& viewport);
        
        void scissorView(const Viewport& viewport);
        
        void clearView(const Color clearColor, const uint32_t clearBits);
        
		void renderQuad();

		Shader getShader(MaterialSettingComBase* matCom);

		Shader getShader(std::string shaderName);

		void renderSkybox(Object objCamera);

		void ssaoPass(Object objCamera, std::string gBufferAliasName, std::string colorBufferAliasName, size_t winWidth, size_t winHeight);

		void deferredLightingPass(Object objCamera, std::string gBufferAliasName, size_t winWidth, size_t winHeight);
        
        void uploadLights(Shader shader);
       
        void renderColorBufferDebug(std::string colorBufferAliasName, size_t winWidth, size_t winHeight);
        
        void renderGBufferDebug(std::string gBufferAliasName, size_t winWidth, size_t winHeight);
	};

};

#endif
