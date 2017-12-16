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

	class UpdateBatchEvent : public ecs::Event<UpdateBatchEvent> {
	public:
		UpdateBatchEvent(const ecs::Object obj, bool recursive) :
			obj(obj),
			recursive(recursive)
		{}
		const ecs::Object obj;
		bool recursive;
	};
	
	class CreateBVHEvent : public ecs::Event<CreateBVHEvent> {
	public:
		CreateBVHEvent(ecs::Object objScene) :
			objScene(objScene)
		{}
		ecs::Object objScene;
	};
};

#endif


