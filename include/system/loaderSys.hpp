#ifndef RENDERER_SYSTEM_LOADER_HPP
#define RENDERER_SYSTEM_LOADER_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"
#include "com/meshes.hpp"


using json = nlohmann::json;
using namespace ecs;

namespace renderer {
	class LoaderSystem : public System<LoaderSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr) override;

		virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override {}
		
		void loadSceneObjects(ObjectManager &objMgr, EventManager &evtMgr, std::string modelsDir, json &config);

		void loadTextures(EventManager &evtMgr, std::string texDir, json &config);
        
        void loadShaders(EventManager &evtMgr, std::string shaderDir, json &config);
        
        void loadMaterials(EventManager &evtMgr, json &config);

		void loadMesh(const std::string &filename, Meshes& meshes);
	};

};

#endif
