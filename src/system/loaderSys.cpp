#include "stdafx.h"
#include "system/loaderSys.hpp"
#include "com/bufferCom.hpp"
#include "com/glcommon.hpp"
#include "com/shader.hpp"
#include "com/spatialData.hpp"
#include "com/animation.hpp"
#include "com/cameraCom.hpp"
#include "com/mesh.hpp"
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
#include "event/spatialEvent.hpp"
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
        string animationSubDir = config["animationDir"];

		Object obj = m_objMgr->create(); // singleTon, manage kinds of resources
		obj.addComponent<RenderMode>();
		obj.addComponent<StaticRenderQueueCom>();
		obj.addComponent<DynamicRenderQueueCom>();
		obj.addComponent<KeyState>();
        obj.addComponent<TextureDict>();
        obj.addComponent<ShaderProgramSet>();
        obj.addComponent<MaterialSet>();
		obj.addComponent<MeshSet>();
		obj.addComponent<MeshBuffersSet>();
		obj.addComponent<GBufferDictCom>();
		obj.addComponent<ColorBufferDictCom>();
		obj.addComponent<InstanceBufferDictCom>();
        obj.addComponent<AnimationDataSet>();
        m_evtMgr->emit<GlobalComAddedEvent>();
        
		
        auto shadowMapSetting = obj.addComponent<ShadowMapSetting>();
        shadowMapSetting->shaderSetting = {
            { LightType::Dir, {
                { ShadowType::Standard, "standardShadowMap" },
                { ShadowType::VSM, "varianceShadowMap" }
            }},
            { LightType::Point, {
                { ShadowType::Standard, "standardShadowMap" },
                { ShadowType::VSM, "varianceShadowMap" }
            }},
            { LightType::Spot, {
                {ShadowType::Standard, "standardShadowMap" },
                {ShadowType::VSM, "varianceShadowMap" }
            }}
        };
		auto gSettingCom = obj.addComponent<GlobalSettingCom>();
        gSettingCom->setValue("depthBias", 0.5f);
		gSettingCom->setValue("normalOffset", 0.0f);
		gSettingCom->setValue("diskFactor", 3.0f);
		gSettingCom->setValue("pointLightConstant", 1.0f);
		gSettingCom->setValue("pointLightLinear", 0.0014f);
		gSettingCom->setValue("pointLightQuad", 0.000007f);
		gSettingCom->setValue("enableSSAO", json(true));
		gSettingCom->setValue("enableSMAA", json(true));
		gSettingCom->setValue("enableToneMapping", json(true));
		gSettingCom->setValue("enableGamma", json(true));
		gSettingCom->setValue("debugShadow", json(false));
		gSettingCom->setValue("bvhDepth", 0);

		CreateGlobalQuadObject();
		loadSkyboxMesh();
		loadWireframeBoxMesh();
		loadOuterBoxMesh();

		Object objCamera = m_objMgr->create();
		auto com = objCamera.addComponent<PerspectiveCameraView>(45.0f, (float)winWidth / (float)winHeight, 0.1f, 10000.0f);
        //com->eye = Vector3dF(156.0f, 116.0f, 143.0f);
        //com->SetFrontVector({30.0f, -15.0f, -30.0f});
        com->eye = Vector3dF(0.0f, 0.0f, 3.0f);
        com->SetFrontVector({0.0f, 0.0f, -1.0f});

        loadTextures(assetsDir + texSubDir, config);
		loadSkyboxes(assetsDir + skyboxSubDir, config);
        loadShaders(assetsDir + shaderSubDir, config);
        loadMaterials(config);
		loadLights(config);
        m_evtMgr->emit<LoadAnimationEvent>(assetsDir + animationSubDir, config["animations"]);
		Object objRoot = loadSceneObjects(config, config["sceneRoot"]);
		objRoot.addComponent<RootNodeTag>();
		m_evtMgr->emit<UpdateBatchEvent>(objRoot, true);
		m_evtMgr->emit<CreateBVHEvent>(objRoot, objRoot);
		m_evtMgr->emit<CreateKdTreeEvent>(objRoot, objRoot);
		

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
            BufType::Tex, 0, GL_LINEAR, "pingBuf");
        
        m_evtMgr->emit<CreateColorBufferEvent>(
            winWidth, winHeight,
			GL_RGBA16F, GL_RGBA, GL_FLOAT,
            BufType::Tex, 0, GL_LINEAR, "pongBuf");
        
        m_evtMgr->emit<CreateGBufferEvent>(winWidth, winHeight, "main");
        
        m_evtMgr->emit<CreateNoiseTextureEvent>("ssaoNoise");
      
	}

	void LoaderSystem::loadSkyboxMesh() {
		auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
		std::string name = "skybox";
		auto it = meshSetCom->alias2id.find(name);
		if (it != meshSetCom->alias2id.end()) {
			return;
		}
		MeshID meshID;
		Mesh& mesh = meshSetCom->newMesh(name, meshID);
		generateSkyBoxMesh(mesh);
		m_evtMgr->emit<CreateSkyboxBufferEvent>(meshID);
		printf("create skybox mesh, ID:%d\n", meshID);
	}

	void LoaderSystem::loadWireframeBoxMesh() {
		auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
		MeshID meshID;
		Mesh& mesh = meshSetCom->newMesh("wfbox", meshID);
		generateWireframeBoxMesh(mesh);
		mesh.meshes[0].meshType = MeshType::Lines;
		m_evtMgr->emit<CreateMeshBufferEvent>(meshID);
	}

	void LoaderSystem::loadOuterBoxMesh() {
		auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
		MeshID meshID;
		Mesh& mesh = meshSetCom->newMesh("outerbox", meshID);
		generateTriBoxMesh(mesh, true);
		m_evtMgr->emit<CreateMeshBufferEvent>(meshID);
	}
	void LoaderSystem::CreateGlobalQuadObject() {
		auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
		MeshID meshID;
		std::string name = "quad";
		auto it = meshSetCom->alias2id.find(name);
		if (it != meshSetCom->alias2id.end()) {
			return;
		}
		Mesh& mesh = meshSetCom->newMesh(name, meshID);
		mesh.meshes.emplace_back();
		SubMesh& subMesh = mesh.meshes[0];
		Vertex v;
		v.position = { -1.0f, 1.0f, 0.0f }; // Left Top
		v.texCoords = { 0.0f, 1.0f };
		subMesh.vertices.push_back(v);
		v.position = { -1.0f, -1.0f, 0.0f }; // Left Bottom
		v.texCoords = { 0.0f, 0.0f };
		subMesh.vertices.push_back(v);
		v.position = { 1.0f, -1.0f, 0.0f }; // Right Bottom
		v.texCoords = { 1.0f, 0.0f };
		subMesh.vertices.push_back(v);
		v.position = { 1.0f, 1.0f, 0.0f }; // Right Top
		v.texCoords = { 1.0f, 1.0f };
		subMesh.vertices.push_back(v);
		subMesh.indexes = {
			0, 1, 2,
			0, 2, 3 };
		m_evtMgr->emit<CreateMeshBufferEvent>(meshID);
		printf("create quad mesh, ID:%d\n", meshID);
	}

	void LoaderSystem::loadLights(const json &config) {
		int count = 0;
		for (auto lightInfo : config["light"])
		{
			std::string type = lightInfo["type"];
			if (type != "PointLight" && type != "DirLight" && type != "SpotLight") {
				continue;
			}
			Object obj = m_objMgr->create();
			auto lightCommon = obj.addComponent<LightCommon>(
				parseColor(lightInfo["ambient"]),
				parseColor(lightInfo["diffuse"]),
				parseColor(lightInfo["specular"]),
				lightInfo["intensity"]);
			if (lightInfo["shadowType"].is_string() && lightInfo["shadowType"] == "vsm") {
				lightCommon->shadowType = ShadowType::VSM;
			}
			if (type == "PointLight") {
				auto spatial = lightInfo["spatial"];
				float far_plane = lightInfo["far_plane"];
				loadSpatialData(obj, spatial);
				auto transCom = obj.addComponent<PointLightTransform>();
				transCom->aspect = 1.0f;
				transCom->fovy = 90.0f;
				transCom->n = 1.0f;
				transCom->f = far_plane;
				obj.addComponent<PointLightCom>(
					lightInfo["constant"],
					lightInfo["linear"],
					lightInfo["quadratic"],
					1024);
				obj.addComponent<MeshRef>("outerbox");
				m_evtMgr->emit<EnableLightShadowEvent>(obj);
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
			} else if (type == "DirLight") {
				auto spatial = lightInfo["spatial"];
				loadSpatialData(obj, spatial);
				obj.addComponent<DirLightCom>();
				auto transCom = obj.addComponent<DirLightTransform>();
				float size = lightInfo["size"];
				float near_plane = lightInfo["near_plane"];
				float far_plane = lightInfo["far_plane"];
				transCom->size = size;
				transCom->n = near_plane;
				transCom->f = far_plane;
				m_evtMgr->emit<EnableLightShadowEvent>(obj);
			} else if (type == "SpotLight") {
				float cutOff = lightInfo["cutOff"];
				float near_plane = lightInfo["near_plane"];
				float far_plane = lightInfo["far_plane"];
				float outerCutOff = lightInfo["outerCutOff"];
				auto transCom = obj.addComponent<SpotLightTransform>();
				transCom->aspect = 1.0f;
				transCom->fovy = 90.0f;
				transCom->n = near_plane;
				transCom->f = far_plane;
				loadSpatialData(obj, lightInfo["spatial"]);
				auto spatialData = obj.component<SpatialData>();
				Vector3dF dir(1.0f, 0.0f, 0.0f);
				if (!lightInfo["direction"].is_null()) {
					auto direction = lightInfo["direction"];
					dir = Vector3dF{ (float)direction[0], (float)direction[1], (float)direction[2] };
				}
				else if (!lightInfo["target"].is_null()) {
					auto target = lightInfo["target"];
					dir = Vector3dF{ (float)target[0], (float)target[1], (float)target[2] } - spatialData->pos;
				}
				obj.addComponent<SpotLightCom>(
					dir.Normalize(), DegreeF(cutOff).ToRadian(), DegreeF(outerCutOff).ToRadian());
				m_evtMgr->emit<EnableLightShadowEvent>(obj);
            } else {
                obj.destroy();
				continue;
            }
			obj.addComponent<LightTag>();
			m_evtMgr->emit<AddLightEvent>(obj);
		}
	}

	Object LoaderSystem::loadSceneObjects(const json &config, const json &objInfo) {
        auto aniDataSet = m_objMgr->getSingletonComponent<AnimationDataSet>();
		Object objScene = m_objMgr->create();
		auto sgNode = objScene.addComponent<SceneGraphNode>();
		auto spatial = objInfo["spatial"];
		loadSpatialData(objScene, spatial);
		bool hasModel = objInfo["model"].is_string();
		if (hasModel) {
			std::string filename = objInfo["model"];
            if (endsWith(filename, std::string(".obj"))) {
                MeshID meshID = loadMesh(config, filename);
                objScene.addComponent<MeshRef>(meshID);
                objScene.addComponent<ReceiveLightTag>();
                objScene.addComponent<RenderableTag>();
                printf("scene obj with mesh, ID: %d, meshID: %d \n", (int)objScene.ID(), (int)meshID);
            } else {
                AnimationData& data = aniDataSet->getAnimationData("test");
                auto id = aniDataSet->getAnimationDataID("test");
                auto com = objScene.addComponent<AnimationCom>(id, data);
                com->curAniName = "run";
            }
		}
		if (objInfo["static"].is_boolean() && bool(objInfo["static"]) == true) {
			objScene.addComponent<StaticObjTag>();
		}
		else {
			objScene.addComponent<DynamicObjTag>();
		}
		objScene.addComponent<MotionCom>();
		/*
		{
			ActionData data;
			data.repeat = -1;
			//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(-1.0f, 0.0f, 0.0f)));
			//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(1.0f, 0.0f, 0.0f)));
			data.actions.push_back(std::make_shared<RotateByAction>(1.0f, DegreeF(0.0f), DegreeF(90.0f), DegreeF(90.0f)));
			data.actions.push_back(std::make_shared<RotateByAction>(1.0f, DegreeF(0.0f), DegreeF(90.0f), DegreeF(90.0f), true));
			m_evtMgr->emit<AddActionEvent>(objScene, "rotate", data);
		}
		*/
		for (auto childObjInfo : objInfo["children"])
		{
			for (int i = 0; i < 20; i++) {
				Object childObj = loadSceneObjects(config, childObjInfo);
				sgNode->children.push_back(childObj.ID());
				auto spatialData = childObj.component<SpatialData>();
				spatialData->pos = {
					randomFloat() * 30.0f,
					0.0f,
					randomFloat() * 30.0f
				};
				m_evtMgr->emit<UpdateSpatialDataEvent>(childObj);
			}
		}

		return objScene;
	}

	/*
	objScene.addComponent<MotionCom>();
	{
		ActionData data;
		data.repeat = 1;
		//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(-1.0f, 0.0f, 0.0f)));
		//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(1.0f, 0.0f, 0.0f)));
		data.actions.push_back(std::make_shared<RotateByAction>(1.0f, DegreeF(0.0f), DegreeF(90.0f), DegreeF(90.0f)));
		data.actions.push_back(std::make_shared<RotateByAction>(1.0f, DegreeF(0.0f), DegreeF(90.0f), DegreeF(90.0f), true));
		// m_evtMgr->emit<AddActionEvent>(objScene, "rotate", data);
	}
	{
		ActionData data;
		data.repeat = -1;
		//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(-1.0f, 0.0f, 0.0f)));
		//data.actions.push_back(std::make_shared<MoveByAction>(0.5f, Vector3dF(1.0f, 0.0f, 0.0f)));
		data.actions.push_back(std::make_shared<MoveByAction>(1.0f, Vector3dF{ 3.0f, 0.0f, 0.0f }));
		data.actions.push_back(std::make_shared<MoveByAction>(1.0f, Vector3dF{ -3.0f, 0.0f, 0.0f }));
		//m_evtMgr->emit<AddActionEvent>(objScene, "move", data);
	}*/

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
			objSkybox.addComponent<MeshRef>("skybox");

			string aliasname = it.key();
			json& data = it.value();
			std::vector<std::string> filenames;
			auto files = data["files"];
			for (auto it2 = files.begin(); it2 != files.end(); it2++) {
				filenames.push_back(it2.value());
			}
			size_t channels = data["channels"];
			m_evtMgr->emit<LoadCubemapEvent>(skyboxSubDir, filenames, aliasname, channels);
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

	MeshID LoaderSystem::loadMesh(const json &config, const std::string &filename)
	{
		auto meshSetCom = m_objMgr->getSingletonComponent<MeshSet>();
		auto it = meshSetCom->alias2id.find(filename);
		if (it != meshSetCom->alias2id.end()) {
			return it->second;
		}
		MeshID meshID;
		Mesh& mesh = meshSetCom->newMesh(filename, meshID);
		Assimp::Importer importer;
		string assetsDir = config["assetsDir"];
		string modelsDir = config["modelsDir"];
		const aiScene* scene = importer.ReadFile(assetsDir + modelsDir + filename,
			// aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_PreTransformVertices |
			aiProcess_RemoveRedundantMaterials |
			aiProcess_GenSmoothNormals |
			aiProcess_OptimizeMeshes |
#if !defined(__APPLE__)
            aiProcess_OptimizeGraph | // not suppoted on Mac OS
#endif
			aiProcess_SortByPType);
		// If the import failed, report it
		if (!scene)
		{
			cout << importer.GetErrorString() << endl;
			return 0;
		}
		std::vector<MaterialSettingID> settingIDs;
		string texSubDir = config["texSubDir"];
		m_evtMgr->emit<LoadAiMaterialEvent>(scene->mNumMaterials,
			scene->mMaterials, 
			assetsDir + texSubDir,
			settingIDs);
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[i];
			if (aimesh->mNumVertices <= 0) {
				continue;
			}
			mesh.meshes.emplace_back();
			SubMesh& subMesh = *std::prev(mesh.meshes.end());
			mesh.settingIDs.push_back(settingIDs[aimesh->mMaterialIndex]);
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
				subMesh.vertices.push_back(v);
			}
			for (GLuint i = 0; i < aimesh->mNumFaces; i++)
			{
				aiFace face = aimesh->mFaces[i];
				// mNumIndices must be 3 because of aiProcess_Triangulate
				uint32_t idx0 = face.mIndices[0];
				uint32_t idx1 = face.mIndices[1];
				uint32_t idx2 = face.mIndices[2]; 
				
				subMesh.indexes.push_back(idx0);
				subMesh.indexes.push_back(idx1);
				subMesh.indexes.push_back(idx2);

				Vertex& v1 = subMesh.vertices[idx0];
				Vertex& v2 = subMesh.vertices[idx1];
				Vertex& v3 = subMesh.vertices[idx2];

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
			subMesh.InitBound();
		}
		mesh.InitBound();
		std::cout << "[LoaderSystem] loadMesh:" << filename  << ", numMaterials:" << scene->mNumMaterials << std::endl;
		m_evtMgr->emit<CreateMeshBufferEvent>(meshID);
		return meshID;
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
