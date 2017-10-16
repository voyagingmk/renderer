#ifndef RENDERER_EVENT_LOADER_HPP
#define RENDERER_EVENT_LOADER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {
    
	class LoadFinishEvent: public ecs::Event<CreateMeshBufferEvent> {
	public:
		LoadFinishEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};


};

#endif