#include "stdafx.h"
#include "realtime/glutils.hpp"
#include "system/renderSys.hpp"
#include "com/sdlContext.hpp"
#include "com/meshes.hpp"
#include "com/cameraCom.hpp"
#include "com/bufferCom.hpp"
#include "com/lightCom.hpp"
#include "com/miscCom.hpp"
#include "system/spatialSys.hpp"
#include "event/materialEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/spatialEvent.hpp"
#include "event/bufferEvent.hpp"
#include "event/textureEvent.hpp"
#include "event/lightEvent.hpp"
#include "utils/helper.hpp"
#include "AreaTex.h"
#include "SearchTex.h"


using namespace std;


namespace renderer {
	void RenderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("RenderSystem init\n");
		evtMgr.on<RenderSceneEvent>(*this);
		evtMgr.on<CameraMoveEvent>(*this);
	}

	// renderpipe loop, could move to another system
	void RenderSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		auto context = objMgr.getSingletonComponent<SDLContext>();
		auto gSettingCom = objMgr.getSingletonComponent<GlobalSettingCom>();
		Object objCamera = objMgr.getSingletonComponent<PerspectiveCameraView>().object();
		if (!context.valid()) {
			return;
		}
		auto screenViewport = std::make_tuple(0, 0, context->width, context->height);
		setViewport(screenViewport);
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		/*----- first-pass: deferred rendering-----*/

		//  geometry pass
		evtMgr.emit<UseGBufferEvent>("main");
		Shader gBufferShader = getShader("gBuffer");
		evtMgr.emit<RenderSceneEvent>(
			objCamera,
			screenViewport,
			Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			&gBufferShader);
		evtMgr.emit<UnuseGBufferEvent>("main");
		// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// ssao pass
		ssaoPass(objCamera, "main", "ssao", context->width, context->height);
		ssaoBlurPass(objCamera, "ssao", "ssaoBlur", context->width, context->height);

		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();

		updateShadowMapPass("main", objCamera);

		// lighting pass
		deferredLightingPass("core", objCamera, "main", context->width, context->height);
		
		//renderColorBuffer("core", context->width, context->height);
		//SDL_GL_SwapWindow(context->win);
		//return;
		CheckGLError;

		//setViewport(std::make_tuple(0, 0, context->width, context->height));
	   // clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
	   // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	   /*----- first-pass end -----*/

		ColorBufferRef& coreBuf = colorBufferCom->dict["core"];
		ColorBufferRef& finalBuf = colorBufferCom->dict["final"];

		evtMgr.emit<UseColorBufferEvent>("final");
		renderColorBuffer("core", context->width, context->height, true, true);
		evtMgr.emit<UnuseColorBufferEvent>("final");

		evtMgr.emit<CopyGBufferDepth2ColorBufferEvent>("main", "final");// 画skybox需要GBuffer的深度信息

		evtMgr.emit<UseColorBufferEvent>("final");
		// skybox pass
		CheckGLError;
		renderSkybox("", objCamera, screenViewport);
		renderLightObjects("", objCamera, screenViewport);
		evtMgr.emit<UnuseColorBufferEvent>("final");

		bool noToneMapping = !gSettingCom->get1b("enableToneMapping");
		bool noGamma = !gSettingCom->get1b("enableGamma");

		if (gSettingCom->get1b("enableSMAA") == true) {
			// debug edge detect
			{
				evtMgr.emit<UseColorBufferEvent>("edge");
				Shader edgeDetect = getShader("smaaEdgeDetect");
				edgeDetect.use();
				edgeDetect.set2f("imgSize", context->width, context->height);
				clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
					GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				setViewport(screenViewport);
				m_evtMgr->emit<ActiveTextureByIDEvent>(edgeDetect, "colorTex", 0, finalBuf.tex);
				renderQuad();
				evtMgr.emit<UnuseColorBufferEvent>("edge");
			}

			static unsigned int area_tex = 0;
			static unsigned int search_tex = 0;
			if (area_tex == 0) {
				glGenTextures(1, &area_tex);
				glBindTexture(GL_TEXTURE_2D, area_tex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, (GLsizei)AREATEX_WIDTH, (GLsizei)AREATEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, areaTexBytes);
			}
			if (search_tex == 0) {
				glGenTextures(1, &search_tex);
				glBindTexture(GL_TEXTURE_2D, search_tex);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLsizei)SEARCHTEX_WIDTH, (GLsizei)SEARCHTEX_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, searchTexBytes);
			}
			{
				evtMgr.emit<UseColorBufferEvent>("weight");
				Shader smaaWeight = getShader("smaaWeight");
				smaaWeight.use();
				smaaWeight.set2f("imgSize", context->width, context->height);
				ColorBufferRef& edgeBuf = colorBufferCom->dict["edge"];
				clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
					GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				setViewport(screenViewport);
				m_evtMgr->emit<ActiveTextureByIDEvent>(smaaWeight, "edgesTex", 0, edgeBuf.tex);
				m_evtMgr->emit<ActiveTextureByIDEvent>(smaaWeight, "areaTex", 1, area_tex);
				m_evtMgr->emit<ActiveTextureByIDEvent>(smaaWeight, "searchTex", 2, search_tex);
				renderQuad();
				evtMgr.emit<UnuseColorBufferEvent>("weight");
			}
			{
				evtMgr.emit<UseColorBufferEvent>("core");
				Shader smaaBlending = getShader("smaaBlending");
				smaaBlending.use();
				smaaBlending.set2f("imgSize", context->width, context->height);
				ColorBufferRef& weightBuf = colorBufferCom->dict["weight"];
				m_evtMgr->emit<ActiveTextureByIDEvent>(smaaBlending, "colorTex", 0, finalBuf.tex);
				m_evtMgr->emit<ActiveTextureByIDEvent>(smaaBlending, "blendTex", 1, weightBuf.tex);
				clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
					GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				setViewport(screenViewport);
				renderQuad();
				evtMgr.emit<UnuseColorBufferEvent>("core");
				renderColorBuffer("core", context->width, context->height, noGamma, noToneMapping);
			}
		} else {
			renderColorBuffer("final", context->width, context->height, noGamma, noToneMapping);
		}
		// renderColorBuffer("core", context->width, context->height);
		// renderGBufferDebug("main", context->width, context->height);
		CheckGLError;
		m_evtMgr->emit<DrawUIEvent>();
		SDL_GL_SwapWindow(context->win);
	}

	void RenderSystem::receive(const CameraMoveEvent &evt) {

	}
    
    void RenderSystem::setViewport(const Viewport& viewport) {
        glViewport(std::get<0>(viewport),
                   std::get<1>(viewport),
                   std::get<2>(viewport),
                   std::get<3>(viewport));
    }
    
    void RenderSystem::scissorView(const Viewport& viewport) {
        glScissor(std::get<0>(viewport),
                   std::get<1>(viewport),
                   std::get<2>(viewport),
                   std::get<3>(viewport));
    }
    
    void RenderSystem::clearView(const Color clearColor, const uint32_t clearBits) {
        glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
        glClear(clearBits);
    }
    
	void RenderSystem::receive(const RenderSceneEvent &evt) {
		CheckGLError;
		auto matSetCom = m_objMgr->getSingletonComponent<MaterialSet>();
		glEnable(GL_DEPTH_TEST);
        setViewport(evt.viewport);
        clearView(evt.clearColor, evt.clearBits);
		Shader shader;

		if (evt.shader != nullptr) {
			shader = *evt.shader;
			shader.use();
		}

		// TODO: sort by material
		for (const Object obj : m_objMgr->entities<
             Meshes, MaterialCom, SpatialData,
             ReceiveLightTag,
             MeshBuffersCom>()) {
			auto matCom = obj.component<MaterialCom>();
			CheckGLError; 
			m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
			CheckGLError; 
			m_evtMgr->emit<UploadMatrixToShaderEvent>(obj, shader);
			CheckGLError; 
			auto meshBufferCom = obj.component<MeshBuffersCom>();
			for (auto meshBuffer: meshBufferCom->buffers) {
				auto settingID = matCom->settingIDs[meshBuffer.matIdx];	
				auto setting = matSetCom->settings[settingID];
				if (evt.shader == nullptr) {
					shader = getShader(setting);
					shader.use();
				}
				m_evtMgr->emit<ActiveMaterialEvent>(settingID, shader);
				m_evtMgr->emit<DrawOneMeshBufferEvent>(meshBuffer);
				m_evtMgr->emit<DeactiveMaterialEvent>(settingID);
			}
			CheckGLError;
			CheckGLError;
		}
		CheckGLError;
	}

	Shader RenderSystem::getShader(MaterialSettingComBase* com) {
		auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
		return Shader(spSetCom->alias2HDL[com->shaderName]);
	}

	Shader RenderSystem::getShader(std::string shaderName) {
		auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
		return Shader(spSetCom->alias2HDL[shaderName]);
	}

    void RenderSystem::renderQuad() {
        for (auto obj : m_objMgr->entities<GlobalQuadTag>()) {
			m_evtMgr->emit<DrawMeshBufferEvent>(obj);
		}
	}
    
	void RenderSystem::renderSkybox(std::string colorBufferAliasName, Object objCamera, Viewport viewport) {
		m_evtMgr->emit<UseColorBufferEvent>(colorBufferAliasName);
		setViewport(viewport);
		for (const Object obj : m_objMgr->entities<GlobalSkyboxTag>()) {
			// glDisable(GL_CULL_FACE);
			// draw skybox as last
			glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			Shader skyboxShader = getShader("skybox");
			skyboxShader.use();
			m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, skyboxShader);
			m_evtMgr->emit<ActiveTextureEvent>(skyboxShader, "skybox", 0, "alps");
			m_evtMgr->emit<DrawMeshBufferEvent>(obj);
			glDepthFunc(GL_LESS); // set depth function back to default
								  // glEnable(GL_CULL_FACE);
			checkGLError;
			break;
		}
		m_evtMgr->emit<UnuseColorBufferEvent>(colorBufferAliasName);
	}
	
	void RenderSystem::ssaoPass(Object objCamera, std::string gBufferAliasName, std::string ssaoBuffer, size_t winWidth, size_t winHeight) {
		m_evtMgr->emit<UseColorBufferEvent>(ssaoBuffer);
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT);
		Shader shader = getShader("ssao");
		shader.use();
		m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, shader);
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& buf = gBufferCom->dict[gBufferAliasName];
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gPosition", 0, buf.posTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gNormal", 1, buf.normalTexID);
		m_evtMgr->emit<ActiveTextureEvent>(shader, "texNoise", 2, "ssaoNoise");
		shader.set2f("screenSize", float(winWidth), float(winHeight));
		
        static std::vector<Vector3dF> ssaoKernel;
        if (ssaoKernel.size() == 0) {
            generateSampleKernel(ssaoKernel);
        }
        shader.set3fArray("samples", ssaoKernel);
        CheckGLError;
		renderQuad();
        CheckGLError;
		m_evtMgr->emit<UnuseColorBufferEvent>(ssaoBuffer);
	}

	void RenderSystem::ssaoBlurPass(Object objCamera, std::string ssaoBuffer, std::string ssaoBlurBuffer, size_t winWidth, size_t winHeight) {
		m_evtMgr->emit<UseColorBufferEvent>(ssaoBlurBuffer);
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& buf = colorBufferCom->dict[ssaoBuffer];
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT);
		Shader shader = getShader("ssaoBlur");
		shader.use();
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "ssaoInput", 0, buf.tex.texID);
		CheckGLError;
		renderQuad();
		m_evtMgr->emit<UnuseColorBufferEvent>(ssaoBlurBuffer);
	}

	void RenderSystem::updateShadowMapPass(std::string gBufferAliasName, Object objCamera) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& gBuf = gBufferCom->dict[gBufferAliasName];
		// Point Light Pass
		for (auto obj: m_objMgr->entities<PointLightCom, PointLightTransform, SpatialData>()) {
			auto lightCom = obj.component<PointLightCom>();
			auto transCom = obj.component<PointLightTransform>();
			auto spatialDataCom = obj.component<SpatialData>();
			auto bufAliasname = "lightDepth" + std::to_string(obj.ID());
			if (colorBufferCom->dict.find(bufAliasname) == colorBufferCom->dict.end()) {
				continue;
			}
			ColorBufferRef& shadowBuf = colorBufferCom->dict[bufAliasname];
			m_evtMgr->emit<UseColorBufferEvent>(bufAliasname);
			CheckGLError;
			Shader pointShadowDepthShader = getShader("pointShadowDepth");
			pointShadowDepthShader.use();
			pointShadowDepthShader.setMatrixes4f("lightPVs", transCom->lightPVs);
			pointShadowDepthShader.set1f("far_plane", transCom->f);
			pointShadowDepthShader.set3f("lightPos", obj.component<SpatialData>()->pos);
			pointShadowDepthShader.set1f("normalOffset", gSettingCom->get1f("normalOffset", 0.0f));
			CheckGLError;
			glCullFace(GL_FRONT);
			m_evtMgr->emit<RenderSceneEvent>(
				objCamera,
				std::make_tuple(0, 0, shadowBuf.width, shadowBuf.height),
				Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
				&pointShadowDepthShader);
			m_evtMgr->emit<UnuseColorBufferEvent>(bufAliasname);
			glCullFace(GL_BACK);
			CheckGLError;
		}
		// Directional Light Pass
		for (auto obj : m_objMgr->entities<DirLightCom, DirLightTransform>()) {
			auto transCom = obj.component<DirLightTransform>();
			auto bufAliasname = "lightDepth" + std::to_string(obj.ID());
			if (colorBufferCom->dict.find(bufAliasname) == colorBufferCom->dict.end()) {
				continue;
			}
			ColorBufferRef& shadowBuf = colorBufferCom->dict[bufAliasname];
			m_evtMgr->emit<UseColorBufferEvent>(bufAliasname);
			CheckGLError;
			Shader dirLightDepthShader = getShader("dirLightDepth");
			dirLightDepthShader.use();
			dirLightDepthShader.setMatrix4f("lightPV", transCom->lightPV);
			auto lightPV = transCom->lightPV;
			// dirLightShadowShader.set3f("lightPos", obj.component<SpatialData>()->pos);
			CheckGLError;
			m_evtMgr->emit<RenderSceneEvent>(
				objCamera,
				std::make_tuple(0, 0, shadowBuf.width, shadowBuf.height),
				Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
				&dirLightDepthShader);
			m_evtMgr->emit<UnuseColorBufferEvent>(bufAliasname);
		}
	}

	void RenderSystem::deferredLightingPass(std::string colorBufferAliasName, Object objCamera, std::string gBufferAliasName, size_t winWidth, size_t winHeight) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		m_evtMgr->emit<UseColorBufferEvent>(colorBufferAliasName);
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& buf = gBufferCom->dict[gBufferAliasName];
		Shader shader = getShader("deferredShading");
		shader.use();
		m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, shader);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gPosition", 0, buf.posTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gNormal", 1, buf.normalTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gAlbedo", 2, buf.albedoTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gPBR", 3, buf.pbrTexID);
		//ColorBufferRef& ssaoBuf = colorBufferCom->dict["ssaoBlur"];
		//m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "ssao", 4, ssaoBuf.tex.texID);
		shader.set1f("depthBias", gSettingCom->get1f("depthBias", 1.0f));
		shader.set1f("diskFactor", gSettingCom->get1f("diskFactor", 3.0f));
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		shader.set1i("begin", 1);
		checkGLError;
		renderQuad();
		shader.set1i("begin", 0);
		for (auto obj : m_objMgr->entities<LightTag>()) {
			uploadLight(shader, obj);
			renderQuad();
		}
		glUseProgram(0);
		checkGLError;
		m_evtMgr->emit<UnuseColorBufferEvent>(colorBufferAliasName);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
    
	void RenderSystem::uploadLight(Shader shader, Object lightObject) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto lightCommon = lightObject.component<LightCommon>();
		shader.set1f("light.intensity", lightCommon->intensity);
		shader.set3f("light.Color", lightCommon->ambient);
		if (lightObject.hasComponent<PointLightCom>()) {
			auto spatialDataCom = lightObject.component<SpatialData>();
			auto lightCom = lightObject.component<PointLightCom>();
			auto transCom = lightObject.component<PointLightTransform>();
			shader.set1i("light.type", 2);
			shader.set3f("light.Position", spatialDataCom->pos);
			shader.set1f("light.far_plane", transCom->f);
			//shader.set1f(("lights[" + std::to_string(i) + "].constant").c_str(), lightCom->constant);
			//shader.set1f(("lights[" + std::to_string(i) + "].Linear").c_str(), lightCom->linear);
			//shader.set1f(("lights[" + std::to_string(i) + "].Quadratic").c_str(), lightCom->quadratic);
		} else if(lightObject.hasComponent<DirLightCom>()) {
			auto spatialDataCom = lightObject.component<SpatialData>();
			auto lightCom = lightObject.component<DirLightCom>();
			auto transCom = lightObject.component<DirLightTransform>();
			shader.set1i("light.type", 1);
			shader.set3f("light.Direction", -(spatialDataCom->pos.Normalize()));
			shader.setMatrix4f("light.lightPV", transCom->lightPV);
		} else if (lightObject.hasComponent<SpotLightCom>()) {
			auto spatialDataCom = lightObject.component<SpatialData>();
			auto lightCom = lightObject.component<SpotLightCom>();
			shader.set1i("light.type", 3);
			shader.set3f("light.Direction", lightCom->direction);
			shader.set3f("light.Position", spatialDataCom->pos);
			shader.set1f("light.cutOff", cos(lightCom->cutOff.radian));
			shader.set1f("light.outerCutOff", cos(lightCom->outerCutOff.radian));
		}
		std::string depthBufName = "lightDepth" + std::to_string(lightObject.ID());
		if (colorBufferCom->dict.find(depthBufName) != colorBufferCom->dict.end()) {
			ColorBufferRef& shadowBuf = colorBufferCom->dict[depthBufName];
			if (shadowBuf.depthTex.type == TexType::CubeMap){
				m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "depthCubeMap", 4, shadowBuf.depthTex);
			}
			else {
				m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "depthMap", 4, shadowBuf.depthTex);
			}
			shader.set1i("light.castShadow", 1);
		}
		else {
			shader.set1i("light.castShadow", 0);
		}
		auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
		float constant = gSettingCom->get1f("pointLightConstant");
		float linear = gSettingCom->get1f("pointLightLinear");
		float quadratic = gSettingCom->get1f("pointLightQuad");
		shader.set1f("light.constant", constant);
		shader.set1f("light.Linear", linear);
		shader.set1f("light.Quadratic", quadratic);
	}

	void RenderSystem::renderLightObjects(std::string colorBufferAliasName, Object objCamera, Viewport viewport) {
		m_evtMgr->emit<UseColorBufferEvent>(colorBufferAliasName); 
		setViewport(viewport);
		// draw light object
		Shader lightShader = getShader("light");
		lightShader.use();
		// TODO: sort by material
		for (auto lightObj : m_objMgr->entities<
			Meshes, PointLightCom, SpatialData,
			MeshBuffersCom>()) {
			m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, lightShader);
			CheckGLError;
			m_evtMgr->emit<UploadMatrixToShaderEvent>(lightObj, lightShader);
			CheckGLError;
			m_evtMgr->emit<DrawMeshBufferEvent>(lightObj);
			CheckGLError;
		}
		m_evtMgr->emit<UnuseColorBufferEvent>(colorBufferAliasName);
	}
    
    void RenderSystem::renderColorBuffer(std::string colorBufferAliasName, size_t winWidth, size_t winHeight, bool noGamma, bool noToneMapping) {
        Shader shader = getShader("screen");
        shader.use();
		shader.set1b("noGamma", noGamma);
		shader.set1b("noToneMapping", noToneMapping);
        auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
        ColorBufferRef& buf = colorBufferCom->dict[colorBufferAliasName];
        clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        setViewport(std::make_tuple(0, 0, winWidth, winHeight));
        m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "texture1", 0, buf.tex.texID);
        renderQuad();
    }
    
    void RenderSystem::renderGBufferDebug(std::string gBufferAliasName, size_t winWidth, size_t winHeight) {
        Shader shader = getShader("screen");
        shader.use();
        auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
        GBufferRef& buf = gBufferCom->dict[gBufferAliasName];
        
        clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        // left-top: position
        setViewport(std::make_tuple(0, winHeight / 2, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "texture1", 0, buf.posTexID);
        renderQuad();
        
        // right-top: normal
        setViewport(std::make_tuple(winWidth / 2, winHeight / 2, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "texture1", 0, buf.normalTexID);
        renderQuad();
        
        // left-bottom: albedo
        setViewport(std::make_tuple(0, 0, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "texture1", 0, buf.albedoTexID);
        renderQuad();
        
        // right-bottom: pbr
        setViewport(std::make_tuple(winWidth / 2, 0, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "texture1", 0, buf.pbrTexID);
        renderQuad();
    }
};
