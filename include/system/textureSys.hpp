#ifndef RENDERER_SYSTEM_TEXTURE_HPP
#define RENDERER_SYSTEM_TEXTURE_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/textureEvent.hpp"


using namespace ecs;

namespace renderer {
	class TextureSystem : public System<TextureSystem>, public Receiver<TextureSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr) override;
		
        virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override {}
        
		void receive(const LoadCubemapEvent &evt);

		void receive(const LoadTextureEvent &evt);
        
        void receive(const CreateNoiseTextureEvent &evt);

		void receive(const DestroyTextureEvent &evt);
		
		void receive(const ActiveTextureByIDEvent &evt);
        
		void receive(const ActiveTextureEvent &evt);
        
        void receive(const DeactiveTextureEvent &evt);
        
        void receive(const CreateDepthTextureEvent &evt);
        
        TexRef CreateDepthTexture(DepthTexType dtType, size_t width, size_t height);
	};


};

#endif
