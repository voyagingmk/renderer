#ifndef RENDERER_SYSTEM_LOADER_HPP
#define RENDERER_SYSTEM_LOADER_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"
#include "com/meshes.hpp"
#include "event/winEvent.hpp"
#include "event/miscEvent.hpp"


using json = nlohmann::json;
using namespace ecs;

namespace renderer {
	class LoaderSystem : public System<LoaderSystem>, public Receiver<LoaderSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr) override;

		virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override {}

		void receive(const LoadConfigEvent &evt);

		void CreateGlobalQuadObject();

		void loadLights(const json &config);

		void loadSceneObjects(std::string modelsDir, const json &config);

		void loadTextures(std::string texDir, const json &config);
        
        void loadShaders(std::string shaderDir, const json &config);
        
        void loadMaterials(const json &config);

		void loadMesh(const std::string &filename, Meshes& meshes);

	private:
		void loadSpatialData(Object obj, const json &config);

	};

};

#endif
