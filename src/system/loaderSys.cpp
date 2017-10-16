#include "stdafx.h"
#include "system/loaderSys.hpp"
#include "com/bufferCom.hpp"
#include "com/glcommon.hpp"
#include "com/shader.hpp"
#include "com/spatialData.hpp"
#include "com/cameraCom.hpp"
#include "com/meshes.hpp"
#include "com/lightCom.hpp"
#include "com/materialCom.hpp"
#include "com/sdlContext.hpp"
#include "com/renderMode.hpp"
#include "com/keyState.hpp"
#include "com/miscCom.hpp"
#include "com/motionCom.hpp"
#include "com/actionCom.hpp"
#include "event/textureEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/materialEvent.hpp"
#include "event/bufferEvent.hpp"
#include "event/actionEvent.hpp"
#include "event/lightEvent.hpp"
#include "importer.hpp"
#include "utils/helper.hpp"

using namespace std;

namespace renderer {
	void LoaderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("LoaderSystem init\n");
		evtMgr.on<LoadConfigEvent>(*this);
	}
	void LoaderSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}
	void LoaderSystem::receive(const LoadConfigEvent &evt) {
		const json& config = evt.config;
		size_t winWidth = config["width"];
		size_t winHeight = config["height"];
		m_evtMgr->emit<SetupSDLEvent>(winWidth, winHeight);

		string assetsDir = config["assetsDir"];
        string texSubDir = config["texSubDir"];
		string skyboxSubDir = config["skyboxSubDir"];
        string shaderSubDir = config["shaderSubDir"];
		string modelsDir = config["modelsDir"];

		Object obj = m_objMgr->create(); // singleTon, manage kinds of resources
		obj.addComponent<RenderMode>();
		obj.addComponent<KeyState>();
        obj.addComponent<TextureDict>();
        obj.addComponent<ShaderProgramSet>();
        obj.addComponent<MaterialSet>();
		obj.addComponent<GBufferDictCom>();
		obj.addComponent<ColorBufferDictCom>();

		Object objCamera = m_objMgr->create();
		auto com = objCamera.addComponent<PerspectiveCameraView>(45.0f, (float)winWidth / (float)winHeight);
        com->eye = Vector3dF(-30.0f, 10.0f, 30.0f);
        com->SetFrontVector({30.0f, 0.0f, -30.0f});
        loadTextures(assetsDir + texSubDir, config);
		loadSkyboxes(assetsDir + skyboxSubDir, config);
        loadShaders(assetsDir + shaderSubDir, config);
        loadMaterials(config);
		loadSceneObjects(assetsDir + modelsDir, config); 
		loadLights(config);

		m_evtMgr->emit<CreateColorBufferEvent>(
			winWidth,
			winHeight,
			GL_RED,
			GL_FLOAT,
			BufType::None,
			0,
			GL_NEAREST,
			"ssao");

		m_evtMgr->emit<CreateColorBufferEvent>(
			winWidth,
			winHeight,
			GL_RGB,
			GL_FLOAT,
			BufType::None,
			0,
			GL_NEAREST,
			"ssaoBlur");

        m_evtMgr->emit<CreateGBufferEvent>(winWidth, winHeight, "main");
        
        m_evtMgr->emit<CreateNoiseTextureEvent>("ssaoNoise");
      
        m_evtMgr->emit<CreateDpethBufferEvent>("shadow", "pointDepth", DepthTexType::CubeMap, 1024);

		CreateGlobalQuadObject();
        
        PointLightTransform c;

	}

	void LoaderSystem::CreateGlobalQuadObject() {
		Object objQuad = m_objMgr->create();
		std::vector<OneMesh> meshes;
		OneMesh mesh;
		Vertex v;
		v.position = { -1.0f, 1.0f, 0.0f }; // Left Top
		v.texCoords = { 0.0f, 1.0f };
		mesh.vertices.push_back(v);
		v.position = { -1.0f, -1.0f, 0.0f }; // Left Bottom
		v.texCoords = { 0.0f, 0.0f };
		mesh.vertices.push_back(v);
		v.position = { 1.0f, -1.0f, 0.0f }; // Right Bottom
		v.texCoords = { 1.0f, 0.0f };
		mesh.vertices.push_back(v);
		v.position = { 1.0f, 1.0f, 0.0f }; // Right Top
		v.texCoords = { 1.0f, 1.0f };
		mesh.vertices.push_back(v);
		mesh.indexes = {
			0, 1, 2,
			0, 2, 3 };
		meshes.push_back(mesh);
		objQuad.addComponent<Meshes>(meshes);
		m_evtMgr->emit<CreateMeshBufferEvent>(objQuad);
		objQuad.addComponent<GlobalQuadTag>();
	}

	void LoaderSystem::loadLights(const json &config) {
		for (auto lightInfo : config["light"])
		{
			Object obj = m_objMgr->create();
			auto spatial = lightInfo["spatial"];
			loadSpatialData(obj, spatial);
			auto transCom = obj.addComponent<PointLightTransform>();
			transCom->aspect = 1.0f;
			transCom->fovy = 90.0f;
			transCom->n = 1.0f;
			transCom->f = 25.0f;
			obj.addComponent<PointLightCom>(
				parseColor(lightInfo["ambient"]),
				parseColor(lightInfo["diffuse"]),
				parseColor(lightInfo["specular"]),
				lightInfo["constant"],
				lightInfo["linear"],
                lightInfo["quadratic"]);
            auto com = obj.addComponent<Meshes>();
            generateOuterBoxMeshes(*com);
            m_evtMgr->emit<AddPointLightEvent>(obj);
			m_evtMgr->emit<CreateMeshBufferEvent>(obj);
		}
	}

	void LoaderSystem::loadSceneObjects(std::string modelsDir, const json &config) {
		for (auto objInfo : config["object"])
		{
			Object obj = m_objMgr->create();
			std::string filename = objInfo["model"];
			auto spatial = objInfo["spatial"];
			int materialID = objInfo["material"];
			bool normalInverse = objInfo["normalInverse"];
			loadSpatialData(obj, spatial);
			Meshes meshes;
			loadMesh(modelsDir + filename, meshes, normalInverse);
			obj.addComponent<Meshes>(meshes);
            obj.addComponent<MaterialCom>(materialID);
			m_evtMgr->emit<CreateMeshBufferEvent>(obj);
            obj.addComponent<ReceiveLightTag>();
            obj.addComponent<MotionCom>();
			{
				ActionData data;
				data.repeat = 1;
				//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(-1.0f, 0.0f, 0.0f)));
				//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(1.0f, 0.0f, 0.0f)));
				data.actions.push_back(std::make_shared<RotateByAction>(1.0f, DegreeF(0.0f), DegreeF(90.0f), DegreeF(90.0f)));
				data.actions.push_back(std::make_shared<RotateByAction>(1.0f, DegreeF(0.0f), DegreeF(90.0f), DegreeF(90.0f), true));
				// m_evtMgr->emit<AddActionEvent>(obj, "rotate", data);
			}
			{
				ActionData data;
				data.repeat = -1;
				//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(-1.0f, 0.0f, 0.0f)));
				//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(1.0f, 0.0f, 0.0f)));
				data.actions.push_back(std::make_shared<MoveByAction>(1.0f, Vector3dF{ 3.0f, 0.0f, 0.0f }));
				data.actions.push_back(std::make_shared<MoveByAction>(1.0f, Vector3dF{ -3.0f, 0.0f, 0.0f }));
				//m_evtMgr->emit<AddActionEvent>(obj, "move", data);
			}
		}
	}

	void LoaderSystem::loadTextures(string texDir, const json &config)
	{
		for (auto texnfo : config["texture"])
		{
			string fileName = texnfo["file"];
			string aliasName = texnfo["alias"];
			size_t channels = texnfo["channels"];
			bool toLinear = texnfo["toLinear"];
			m_evtMgr->emit<LoadTextureEvent>(texDir, fileName.c_str(), aliasName, channels, toLinear);
		}
	}

	void LoaderSystem::loadSkyboxes(string skyboxSubDir, const json &config)
	{
		auto cubemaps = config["skyboxes"];
		for (auto it = cubemaps.begin(); it != cubemaps.end(); it++)
		{
			Object objSkybox = m_objMgr->create();
			m_evtMgr->emit<CreateSkyboxBufferEvent>(objSkybox);

			string aliasname = it.key();
			json& data = it.value();
			std::vector<std::string> filenames;
			auto files = data["files"];
			for (auto it2 = files.begin(); it2 != files.end(); it2++) {
				filenames.push_back(it2.value());
			}
			size_t channels = data["channels"];
			m_evtMgr->emit<LoadCubemapEvent>(skyboxSubDir, filenames, aliasname, channels);

			objSkybox.addComponent<SkyboxCom>(aliasname);
			objSkybox.addComponent<GlobalSkyboxTag>();
		}
	}
	
    
    void LoaderSystem::loadShaders(string shaderDir, const json &config)
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
			LoadShaderEvent::TextureInfo textures;
			auto texturesInfo = shaderInfo["textures"];
			for (json::iterator it = texturesInfo.begin(); it != texturesInfo.end(); it++) {
				textures.push_back(std::make_pair(it.key(), it.value()));
			}
			m_evtMgr->emit<LoadShaderEvent>(shaderDir, names, aliasName, textures);
        }
    }
    
    void LoaderSystem::loadMaterials(const json &config)
    {
        for (auto matInfo : config["material"])
        {
            m_evtMgr->emit<LoadMaterialEvent>(matInfo);
        }
    }

	void LoaderSystem::loadMesh(const std::string &filename, Meshes& meshes, bool normalInverse)
	{
		ImporterAssimp &importer = ImporterAssimp::getInstance();
		importer.Import(filename, meshes, normalInverse);
	}

	void LoaderSystem::loadSpatialData(Object obj, const json &spatial) {
		auto pos = spatial[0];
		auto scale = spatial[1];
		auto o = spatial[2];
		QuaternionF q;
		q.FromEulerAngles(DegreeF((float)o[0]), DegreeF((float)o[1]), DegreeF((float)o[2]));
		obj.addComponent<SpatialData>(
			Vector3dF{ (float)pos[0], (float)pos[1], (float)pos[2] },
			Vector3dF{ (float)scale[0], (float)scale[1], (float)scale[2] },
			q
		);
	}
};
