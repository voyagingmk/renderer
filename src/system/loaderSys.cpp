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

		Object obj = m_objMgr->create(); // singleTon, manage kinds of resources
		obj.addComponent<RenderMode>();
		obj.addComponent<KeyState>();
        obj.addComponent<TextureDict>();
        obj.addComponent<ShaderProgramSet>();
        obj.addComponent<MaterialSet>();
		obj.addComponent<GBufferDictCom>();
		obj.addComponent<ColorBufferDictCom>();
		auto gSettingCom = obj.addComponent<GlobalSettingCom>();
        gSettingCom->setValue("depthBias", 2.0f);
		gSettingCom->setValue("normalOffset", 0.0f);
		gSettingCom->setValue("diskFactor", 3.0f);
		gSettingCom->setValue("pointLightConstant", 1.0f);
		gSettingCom->setValue("pointLightLinear", 0.0014f);
		gSettingCom->setValue("pointLightQuad", 0.000007f);
		gSettingCom->setValue("enableSSAO", json(true));
		gSettingCom->setValue("enableSMAA", json(true));
		gSettingCom->setValue("enableToneMapping", json(true));
		gSettingCom->setValue("enableGamma", json(true));


		Object objCamera = m_objMgr->create();
		auto com = objCamera.addComponent<PerspectiveCameraView>(45.0f, (float)winWidth / (float)winHeight, 0.1f, 10000.0f);
        com->eye = Vector3dF(-19.0, 4.0f, 12.0f);
        com->SetFrontVector({30.0f, 0.0f, -30.0f});
        loadTextures(assetsDir + texSubDir, config);
		loadSkyboxes(assetsDir + skyboxSubDir, config);
        loadShaders(assetsDir + shaderSubDir, config);
        loadMaterials(config);
		loadSceneObjects(config); 
		loadLights(config);

		m_evtMgr->emit<CreateColorBufferEvent>(
			winWidth, winHeight,
			GL_RED, GL_RED, GL_FLOAT,
			BufType::None, 0, GL_NEAREST, "ssao");

		m_evtMgr->emit<CreateColorBufferEvent>(
			winWidth, winHeight,
            GL_RED, GL_RED, GL_FLOAT,
			BufType::None, 0, GL_NEAREST, "ssaoBlur");
       
        m_evtMgr->emit<CreateColorBufferEvent>(
            winWidth, winHeight,
            GL_RGBA8, GL_RGBA, GL_FLOAT,
            BufType::None, 0, GL_LINEAR, "edge");
        
        m_evtMgr->emit<CreateColorBufferEvent>(
            winWidth, winHeight,
            GL_RGBA8, GL_RGBA, GL_FLOAT,
            BufType::None, 0, GL_LINEAR, "weight");

		m_evtMgr->emit<CreateColorBufferEvent>(
            winWidth, winHeight,
			GL_RGBA16F, GL_RGBA, GL_FLOAT,
            BufType::Tex, 0, GL_LINEAR, "core");
        
        m_evtMgr->emit<CreateColorBufferEvent>(
            winWidth, winHeight,
			GL_RGBA16F, GL_RGBA, GL_FLOAT,
            BufType::Tex, 0, GL_LINEAR, "final");
        
        m_evtMgr->emit<CreateGBufferEvent>(winWidth, winHeight, "main");
        
        m_evtMgr->emit<CreateNoiseTextureEvent>("ssaoNoise");
      
        m_evtMgr->emit<CreateDpethBufferEvent>("shadow", "pointDepth", DepthTexType::CubeMap, 1024);

		m_evtMgr->emit<CreateColorBufferEvent>(
			winWidth, winHeight,
			GL_RED, GL_RED, GL_FLOAT,
			BufType::None, 0, GL_LINEAR, "sssm"); // screenspaceshadowmap

		CreateGlobalQuadObject();

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
		int count = 0;
		for (auto lightInfo : config["light"])
		{
			Object obj = m_objMgr->create();
			auto spatial = lightInfo["spatial"];
            float far_plane = lightInfo["far_plane"];
			loadSpatialData(obj, spatial);
			auto transCom = obj.addComponent<PointLightTransform>();
			transCom->aspect = 1.0f;
			transCom->fovy = 90.0f;
			transCom->n = 1.0f;
			transCom->f = far_plane;
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
			obj.addComponent<MotionCom>();
			ActionData data;
			data.repeat = -1;
			//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(-1.0f, 0.0f, 0.0f)));
			//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(1.0f, 0.0f, 0.0f)));
			if (count == 0) {
				data.actions.push_back(std::make_shared<MoveByAction>(2.0f, Vector3dF{ 0.0f, 0.0f, 40.0f }));
				data.actions.push_back(std::make_shared<MoveByAction>(2.0f, Vector3dF{ 0.0f, 0.0f, -40.0f }));
			}
			else {
				data.actions.push_back(std::make_shared<MoveByAction>(3.0f, Vector3dF{ 0.0f, 0.0f, -40.0f }));
				data.actions.push_back(std::make_shared<MoveByAction>(3.0f, Vector3dF{ 0.0f, 0.0f, 40.0f }));
			}
			count++;
		//	m_evtMgr->emit<AddActionEvent>(obj, "move", data);

		}
	}

	void LoaderSystem::loadSceneObjects(const json &config) {
		for (auto objInfo : config["object"])
		{
			Object obj = m_objMgr->create();
			std::string filename = objInfo["model"];
			auto spatial = objInfo["spatial"];
			loadSpatialData(obj, spatial);
			loadMesh(config, obj, filename);
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
			m_evtMgr->emit<LoadTextureEvent>(texDir, fileName, aliasName, channels, toLinear);
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

	void LoaderSystem::loadMesh(const json &config, Object obj, const std::string &filename)
	{
		ComponentHandle<Meshes> comMeshes = obj.addComponent<Meshes>();
		Assimp::Importer importer;
		string assetsDir = config["assetsDir"];
		string modelsDir = config["modelsDir"];
		const aiScene* scene = importer.ReadFile(assetsDir + modelsDir + filename,
			// aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);
		// If the import failed, report it
		if (!scene)
		{
			cout << importer.GetErrorString() << endl;
			return;
		}
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[i];
			if (aimesh->mNumVertices <= 0) {
				continue;
			}
			comMeshes->meshes.push_back(OneMesh());
			OneMesh& mesh = comMeshes->meshes[comMeshes->meshes.size() - 1];
			mesh.matIdx = aimesh->mMaterialIndex;
			std::cout << "[LoaderSystem] matIdx:" << mesh.matIdx << std::endl;
			for (uint32_t i = 0; i < aimesh->mNumVertices; i++)
			{
				Vertex v;
				Vector3dF p, n;
				p.x = aimesh->mVertices[i].x;
				p.y = aimesh->mVertices[i].y;
				p.z = aimesh->mVertices[i].z;
				v.position = p;
				if (aimesh->HasNormals()) {
					// Normals
					n.x = aimesh->mNormals[i].x;
					n.y = aimesh->mNormals[i].y;
					n.z = aimesh->mNormals[i].z;
				}
				v.normal = n;
				/*
				if (aimesh->HasTangentsAndBitangents()) {
					v.tangent.x = aimesh->mTangents[i].x;
					v.tangent.y = aimesh->mTangents[i].y;
					v.tangent.z = aimesh->mTangents[i].z;
				}
				*/
				if (aimesh->HasTextureCoords(0)) {
					Vector2dF uv;
					// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					uv.x = aimesh->mTextureCoords[0][i].x;
					uv.y = aimesh->mTextureCoords[0][i].y;
					v.texCoords = uv;
				} else {
					v.texCoords = Vector2dF(0.0f, 0.0f);
				}
				mesh.vertices.push_back(v);
			}
			for (GLuint i = 0; i < aimesh->mNumFaces; i++)
			{
				aiFace face = aimesh->mFaces[i];
				// mNumIndices must be 3 because of aiProcess_Triangulate
				uint32_t idx0 = face.mIndices[0];
				uint32_t idx1 = face.mIndices[1];
				uint32_t idx2 = face.mIndices[2]; 
				
				mesh.indexes.push_back(idx0);
				mesh.indexes.push_back(idx1);
				mesh.indexes.push_back(idx2);

				Vertex& v1 = mesh.vertices[idx0];
				Vertex& v2 = mesh.vertices[idx1];
				Vertex& v3 = mesh.vertices[idx2];

				Vector3dF edge1 = v2.position - v1.position;
				Vector3dF edge2 = v3.position - v1.position;
				Vector2dF deltaUV1 = v2.texCoords - v1.texCoords;
				Vector2dF deltaUV2 = v3.texCoords - v1.texCoords;
				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
				Vector3dF tangent;
				tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
				tangent = tangent.Normalize();
				v1.tangent = v2.tangent = v3.tangent = tangent;
			}
		}
		std::cout << "[LoaderSystem] loadMesh:" << filename  << ", numMaterials:" << scene->mNumMaterials << std::endl;
		string texSubDir = config["texSubDir"];
		m_evtMgr->emit<LoadAiMaterialEvent>(obj, scene->mNumMaterials, scene->mMaterials, assetsDir + texSubDir);
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
