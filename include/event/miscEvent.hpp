#ifndef RENDERER_EVENT_MISC_HPP
#define RENDERER_EVENT_MISC_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

using json = nlohmann::json;
namespace renderer {

	class LoadConfigEvent: public ecs::Event<LoadConfigEvent> {
	public:
        LoadConfigEvent(const json& config):
			config(config)
        {}
		const json& config;
    };

	class CameraMoveEvent : public ecs::Event<CameraMoveEvent> {
	public:
		CameraMoveEvent(const ecs::Object obj) :
			obj(obj)
		{}
		const ecs::Object obj;
	};

	class StaticBatchEvent : public ecs::Event<StaticBatchEvent> {
	public:
		StaticBatchEvent() {}
	};
	
    
};

#endif


