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
		CreateBVHEvent(ecs::Object objScene, ecs::Object objBVH) :
			objScene(objScene),
			objBVH(objBVH)
		{}
		ecs::Object objScene;
		ecs::Object objBVH;
	};

	class DebugDrawBVHEvent : public ecs::Event<DebugDrawBVHEvent> {
	public:
		DebugDrawBVHEvent(ecs::Object objCamera, ecs::Object objBVH) :
			objCamera(objCamera),
			objBVH(objBVH)
		{}
		ecs::Object objCamera;
		ecs::Object objBVH;
	};

	class CreateKdTreeEvent : public ecs::Event<CreateKdTreeEvent> {
	public:
		CreateKdTreeEvent(ecs::Object objScene, ecs::Object objKdTree) :
			objScene(objScene),
			objKdTree(objKdTree)
		{}
		ecs::Object objScene;
		ecs::Object objKdTree;
	};

	class DebugDrawKdTreeEvent : public ecs::Event<DebugDrawKdTreeEvent> {
	public:
		DebugDrawKdTreeEvent(ecs::Object objCamera, ecs::Object objKdTree) :
			objCamera(objCamera),
			objKdTree(objKdTree)
		{}
		ecs::Object objCamera;
		ecs::Object objKdTree;
	};

};

#endif


