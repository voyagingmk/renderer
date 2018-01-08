#ifndef RENDERER_EVENT_LIGHT_HPP
#define RENDERER_EVENT_LIGHT_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {
    
	class AddLightEvent: public ecs::Event<AddLightEvent> {
	public:
		AddLightEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

	class UpdateLightEvent: public ecs::Event<UpdateLightEvent> {
	public:
		UpdateLightEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

	class EnableLightShadowEvent : public ecs::Event<EnableLightShadowEvent> {
	public:
		EnableLightShadowEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

	class DisableLightShadowEvent : public ecs::Event<DisableLightShadowEvent> {
	public:
		DisableLightShadowEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};
    
    class DrawLightBoundEvent : public ecs::Event<DrawLightBoundEvent> {
    public:
        DrawLightBoundEvent(ecs::Object obj) :
            obj(obj)
        {}
        ecs::Object obj;
    };
    
    
};

#endif
