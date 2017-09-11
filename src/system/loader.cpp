#include "stdafx.h"
#include "system/loader.hpp"
#include "com/glcommon.hpp"
#include "com/shader.hpp"
#include "com/spatialData.hpp"
#include "com/meshes.hpp"
#include "event/textureEvent.hpp"
#include "event/shaderEvent.hpp"

using namespace std;

namespace renderer {
	void LoaderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("LoaderSystem init\n");
        json config = readJson("demo.json");
		string assetsDir = config["assetsDir"];
        string texSubDir = config["texSubDir"];
        string shaderSubDir = config["shaderSubDir"];

		Object obj = objMgr.create(); // manager kinds of resources
        obj.addComponent<TextureDict>();
        obj.addComponent<SPHDLList>();
        obj.addComponent<ShaderProgramAlias>();

        loadTextures(evtMgr, obj, assetsDir + texSubDir, config);
        loadShaders(evtMgr, obj, assetsDir + shaderSubDir, config);
        
        Object objCenter = objMgr.create();
        objCenter.addComponent<SpatialData>(
           Vector3dF{0.0f, 0.0f, 0.0f},
           Vector3dF{0.0f, 0.0f, 0.0f},
           QuaternionF{0.0f, 0.0f, 0.0f, 0.0f}
        );
        auto com = objCenter.addComponent<Meshes>();
        
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
        com->meshes.push_back(mesh);
        
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

};
