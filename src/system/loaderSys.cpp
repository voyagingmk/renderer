#include "stdafx.h"
#include "system/loaderSys.hpp"
#include "com/glcommon.hpp"
#include "com/shader.hpp"
#include "com/spatialData.hpp"
#include "com/cameraCom.hpp"
#include "com/meshes.hpp"
#include "com/materialCom.hpp"
#include "com/sdlContext.hpp"
#include "com/renderMode.hpp"
#include "com/keyState.hpp"
#include "event/textureEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/materialEvent.hpp"
#include "importer.hpp"

using namespace std;

namespace renderer {
	void LoaderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("LoaderSystem init\n");
        json config = readJson("demo.json");
		string assetsDir = config["assetsDir"];
        string texSubDir = config["texSubDir"];
        string shaderSubDir = config["shaderSubDir"];
		string modelsDir = config["modelsDir"];

		Object obj = objMgr.create(); // singleTon, manage kinds of resources
		obj.addComponent<SDLContext>(800, 600);
		obj.addComponent<RenderMode>();
		obj.addComponent<KeyState>();
        obj.addComponent<TextureDict>();
        obj.addComponent<ShaderProgramSet>();
        obj.addComponent<MaterialSet>();

		Object objCamera = objMgr.create();
		objCamera.addComponent<PerspectiveCameraView>();

        loadTextures(evtMgr, obj, assetsDir + texSubDir, config);
        loadShaders(evtMgr, obj, assetsDir + shaderSubDir, config);
        loadMaterials(evtMgr, obj, config);
		loadSceneObjects(objMgr, evtMgr, assetsDir + modelsDir, config);
        
		
        Object objCenter = objMgr.create();
		
        objCenter.addComponent<MaterialCom>(1);

		std::vector<OneMesh> meshes;
        OneMesh mesh;
        Vertex v;
        v.position = {-1.0f, 1.0f, 0.0f}; // Left Top
        v.texCoords = {0.0f, 1.0f};
        mesh.vertices.push_back(v);
        v.position = {-1.0f, -1.0f, 0.0f}; // Left Bottom
        v.texCoords = {0.0f, 0.0f};
        mesh.vertices.push_back(v);
        v.position = {1.0f, -1.0f, 0.0f}; // Right Bottom
        v.texCoords = {1.0f, 0.0f};
        mesh.vertices.push_back(v);
        v.position = {1.0f, 1.0f, 0.0f}; // Right Top
        v.texCoords = {1.0f, 1.0f};
        mesh.vertices.push_back(v);
        mesh.indexes = {
            0, 1, 2,
            0, 2, 3};
		meshes.push_back(mesh);
		objCenter.addComponent<Meshes>(meshes);

		objCenter.addComponent<SpatialData>(
			Vector3dF{ 0.0f, 0.0f, -10.0f },
			Vector3dF{ 0.5f, 0.5f, 0.5f },
			QuaternionF{ 1.0f, 0.0f, 0.0f, 0.0f }
		);
		
		


	}

	void LoaderSystem::loadSceneObjects(ObjectManager &objMgr, EventManager &evtMgr, std::string modelsDir, json &config) {
		for (auto objInfo : config["object"])
		{
			Object obj = objMgr.create();
			std::string filename = objInfo["model"];
			auto spatial = objInfo["spatial"];
			auto pos = spatial[0];
			auto scale = spatial[1];
			auto o = spatial[2];
			obj.addComponent<SpatialData>(
				Vector3dF{ (float)pos[0], (float)pos[1], (float)pos[2] },
				Vector3dF{ (float)scale[0], (float)scale[1], (float)scale[2] },
				QuaternionF{ (float)o[0], (float)o[1], (float)o[2], (float)o[3] }
			);
			Meshes meshes;
			loadMesh(modelsDir + filename, meshes);
			auto com = obj.addComponent<Meshes>(meshes);
            obj.addComponent<MaterialCom>(1);
		}
	}

	void LoaderSystem::loadTextures(EventManager &evtMgr, Object obj, string texDir, json &config)
	{
		for (auto texnfo : config["texture"])
		{
			string fileName = texnfo["file"];
			string aliasName = texnfo["alias"];
			bool hasAlpha = texnfo["hasAlpha"];
			bool toLinear = texnfo["toLinear"];
			evtMgr.emit<LoadTextureEvent>(obj, texDir, fileName.c_str(), aliasName, hasAlpha, toLinear);
		}
	}
    
    void LoaderSystem::loadShaders(EventManager &evtMgr, Object obj, string shaderDir, json &config)
    {
        for (auto shaderInfo : config["shader"])
        {
            std::string aliasName = shaderInfo["alias"];
            ShaderFileNames names({
                {
                    ShaderType::Geometry,
                    shaderInfo["gs"].is_null() ? "" : shaderInfo["gs"].get<std::string>()
                },
				{
                    ShaderType::Vertex,
                    shaderInfo["vs"].get<std::string>()
                },
				{
                    ShaderType::Fragment,
                    shaderInfo["fs"].get<std::string>()
                }
            });
            evtMgr.emit<LoadShaderEvent>(obj, shaderDir, names, aliasName);
        }
    }
    
    void LoaderSystem::loadMaterials(EventManager &evtMgr, Object obj, json &config)
    {
        for (auto matInfo : config["material"])
        {
            evtMgr.emit<LoadMaterialEvent>(obj, matInfo);
        }
    }

	void LoaderSystem::loadMesh(const std::string &filename, Meshes& meshes)
	{
		ImporterAssimp &importer = ImporterAssimp::getInstance();
		meshes.meshes.push_back(OneMesh());
		importer.Import<OneMesh>(filename, meshes.meshes[meshes.meshes.size()- 1]);
	}
};
