#ifndef RENDERER_SYSTEM_LOADER_HPP
#define RENDERER_SYSTEM_LOADER_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"



using json = nlohmann::json;
using namespace ecs;

namespace renderer {
	class LoaderSystem : public System<LoaderSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override {}

		void loadTextures(EventManager &evtMgr, Object obj, std::string& texDir, json &config);
	};

};

#endif