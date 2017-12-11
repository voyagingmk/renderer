#ifndef RENDERER_SYSTEM_LOADER_HPP
#define RENDERER_SYSTEM_LOADER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/mesh.hpp"
#include "event/winEvent.hpp"
#include "event/miscEvent.hpp"


using json = nlohmann::json;
using namespace ecs;

namespace renderer {
	class LoaderSystem : public System<LoaderSystem>, public Receiver<LoaderSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr) override;

		virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
		
		void receive(const LoadConfigEvent &evt);

	private:

		void CreateGlobalQuadObject();

		void loadLights(const json &config);

		Object loadSceneObjects(const json &, const json &);

		void loadTextures(std::string texDir, const json &config);

		void loadSkyboxes(std::string skyboxTexDir, const json &config);
        
        void loadShaders(std::string shaderDir, const json &config);
        
        void loadMaterials(const json &config);

		MeshID loadMesh(const json &config, const std::string &filename);

		void loadSkyboxMesh();

		void loadSpatialData(Object obj, const json &config);

	};

};

#endif
