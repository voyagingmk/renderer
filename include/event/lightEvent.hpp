#ifndef RENDERER_EVENT_LIGHT_HPP
#define RENDERER_EVENT_LIGHT_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {
    
	class AddPointLightEvent: public ecs::Event<AddPointLightEvent> {
	public:
		AddPointLightEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

	class UpdatePointLightEvent: public ecs::Event<UpdatePointLightEvent> {
	public:
		UpdatePointLightEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};
};

#endif