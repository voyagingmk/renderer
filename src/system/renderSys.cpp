#include "stdafx.h"
#include "utils/glutils.hpp"
#include "system/renderSys.hpp"
#include "com/sdlContext.hpp"
#include "com/mesh.hpp"
#include "com/cameraCom.hpp"
#include "com/bufferCom.hpp"
#include "com/lightCom.hpp"
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

		// glCullFace(GL_BACK);

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
		
		//renderGBufferDebug("main", context->width, context->height);
		//m_evtMgr->emit<DrawUIEvent>();
		//SDL_GL_SwapWindow(context->win);
		//return;
		if (gSettingCom->get1b("enableSSAO")) {
			// ssao pass
			// only need G-Buffer
			ssaoPass(objCamera, "main", "ssao", context->width, context->height);
			// optional
			ssaoBlurPass("ssao", "ssaoBlur", context->width, context->height);
		}

		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		
		// TODO: only update shadow map of dynamic lights
		updateShadowMapPass("main", objCamera);
		
		if (gSettingCom->getValue("debugShadow")) {
			renderColorBuffer("lightDepth10", 1024, 1024, true, true);
			m_evtMgr->emit<DrawUIEvent>();
			SDL_GL_SwapWindow(context->win);
			return;
		}

		std::string curSceneBuf = "pingBuf";
		std::string anotherSceneBuf = "pongBuf";

		// lighting pass
		deferredLightingPass(curSceneBuf, objCamera, "main", context->width, context->height);
		

		evtMgr.emit<UseColorBufferEvent>(anotherSceneBuf);
		renderColorBuffer(curSceneBuf, context->width, context->height, true, true);
		evtMgr.emit<UnuseColorBufferEvent>(anotherSceneBuf);
		swap(curSceneBuf, anotherSceneBuf);

		evtMgr.emit<CopyGBufferDepth2ColorBufferEvent>("main", curSceneBuf.c_str());// 画skybox需要GBuffer的深度信息

		evtMgr.emit<UseColorBufferEvent>(curSceneBuf);
		// skybox pass
		
		renderSkybox("", objCamera, screenViewport);
		renderLightObjects("", objCamera, screenViewport);
		evtMgr.emit<UnuseColorBufferEvent>(curSceneBuf);
		
		if (gSettingCom->get1b("enableSSAO")) {
			ssaoApplyPass(curSceneBuf, anotherSceneBuf, "ssaoBlur", context->width, context->height);
			swap(curSceneBuf, anotherSceneBuf);
		}

		bool noToneMapping = !gSettingCom->get1b("enableToneMapping");
		bool noGamma = !gSettingCom->get1b("enableGamma");

		if (gSettingCom->get1b("enableSMAA"))
		{
			setViewport(screenViewport);
			smaaPass(curSceneBuf, anotherSceneBuf);
			swap(curSceneBuf, anotherSceneBuf);
		}
		renderColorBuffer(curSceneBuf, context->width, context->height, noGamma, noToneMapping);
		

		// renderColorBuffer("ssaoBlur", context->width, context->height, true, true);
		// renderGBufferDebug("main", context->width, context->height);

		m_evtMgr->emit<DrawUIEvent>();
		SDL_GL_SwapWindow(context->win);
		// CheckGLError;
	}

	void RenderSystem::receive(const CameraMoveEvent &evt) {

	}

    
    void RenderSystem::scissorView(const Viewport& viewport) {
        glScissor(std::get<0>(viewport),
                   std::get<1>(viewport),
                   std::get<2>(viewport),
                   std::get<3>(viewport));
    }

	Shader RenderSystem::getShader(MaterialSettingBase* com) {
		return getShader(com->shaderName);
	}

	Shader RenderSystem::getShader(std::string shaderName) {
		auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
		return Shader(spSetCom->alias2HDL[shaderName], spSetCom->spHDL2locCache);
	}

    void RenderSystem::renderQuad() {
		m_evtMgr->emit<DrawMeshBufferEvent>("quad", 0);
	}
    
	void RenderSystem::renderSkybox(std::string colorBufferAliasName, Object objCamera, Viewport viewport) {
		m_evtMgr->emit<UseColorBufferEvent>(colorBufferAliasName);
		setViewport(viewport);
		for (const Object objSkybox : m_objMgr->entities<GlobalSkyboxTag>()) {
			// glDisable(GL_CULL_FACE);
			// draw skybox as last
			glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			Shader skyboxShader = getShader("skybox");
			skyboxShader.use();
			m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, skyboxShader);
			m_evtMgr->emit<ActiveTextureEvent>(skyboxShader, "skybox", 0, "alps");
			auto meshName = objSkybox.component<MeshRef>()->meshName;
			m_evtMgr->emit<DrawMeshBufferEvent>(meshName, 0);
			glDepthFunc(GL_LESS); // set depth function back to default
								  // glEnable(GL_CULL_FACE);
			
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
        
		renderQuad();
        
		m_evtMgr->emit<UnuseColorBufferEvent>(ssaoBuffer);
	}

	void RenderSystem::ssaoBlurPass(std::string ssaoBuffer, std::string ssaoBlurBuffer, size_t winWidth, size_t winHeight) {
		m_evtMgr->emit<UseColorBufferEvent>(ssaoBlurBuffer);
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& buf = colorBufferCom->dict[ssaoBuffer];
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT);
		Shader blurShader = getShader("blur");
		blurShader.use();
		m_evtMgr->emit<ActiveTextureByIDEvent>(blurShader, "ssaoInput", 0, buf.tex.texID);
		
		renderQuad();
		m_evtMgr->emit<UnuseColorBufferEvent>(ssaoBlurBuffer);
	}

	void RenderSystem::ssaoApplyPass(std::string inputBuffer, std::string outputBuffer, std::string ssaoBlurBuffer, size_t winWidth, size_t winHeight) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& inputBuf = colorBufferCom->dict[inputBuffer];
		ColorBufferRef& outputBuf = colorBufferCom->dict[outputBuffer];
		ColorBufferRef& ssoaBuf = colorBufferCom->dict[ssaoBlurBuffer];
		m_evtMgr->emit<UseColorBufferEvent>(outputBuffer);
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT);
		Shader shader = getShader("ssaoApply");
		shader.use();
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "img", 0, inputBuf.tex.texID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "ssao", 1, ssoaBuf.tex.texID);
		
		renderQuad();
		m_evtMgr->emit<UnuseColorBufferEvent>(outputBuffer);
	}

	void RenderSystem::updateShadowMapPass(std::string gBufferAliasName, Object objCamera) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& gBuf = gBufferCom->dict[gBufferAliasName];
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
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
			
			Shader pointShadowDepthShader = getShader("pointShadowDepth");
			pointShadowDepthShader.use();
			pointShadowDepthShader.setMatrixes4f("lightPVs", transCom->lightPVs);
			pointShadowDepthShader.set1f("far_plane", transCom->f);
			pointShadowDepthShader.set3f("lightPos", obj.component<SpatialData>()->pos);
			pointShadowDepthShader.set1f("normalOffset", gSettingCom->get1f("normalOffset", 0.0f));
			
			m_evtMgr->emit<RenderSceneEvent>(
				objCamera,
				std::make_tuple(0, 0, shadowBuf.width, shadowBuf.height),
				Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
				&pointShadowDepthShader);
			m_evtMgr->emit<UnuseColorBufferEvent>(bufAliasname);
			
		}
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);
		// Spot Light Pass
		for (auto obj : m_objMgr->entities<SpotLightCom, SpotLightTransform>()) {
			auto lightCommon = obj.component<LightCommon>();
			auto transCom = obj.component<SpotLightTransform>();
			auto bufAliasname = "lightDepth" + std::to_string(obj.ID());
			if (colorBufferCom->dict.find(bufAliasname) == colorBufferCom->dict.end()) {
				continue;
			}
			ColorBufferRef& shadowBuf = colorBufferCom->dict[bufAliasname];
			m_evtMgr->emit<UseColorBufferEvent>(bufAliasname);
			
			auto shadowMapSettingCom = m_objMgr->getSingletonComponent<ShadowMapSetting>();
			auto shaderName = shadowMapSettingCom->shaderSetting[LightType::Dir][lightCommon->shadowType];
			Shader depthShader = getShader(shaderName);
			depthShader.use();
			depthShader.setMatrix4f("lightPV", transCom->lightPV);
			auto lightPV = transCom->lightPV;
			// dirLightShadowShader.set3f("lightPos", obj.component<SpatialData>()->pos);
			
			m_evtMgr->emit<RenderSceneEvent>(
				objCamera,
				std::make_tuple(0, 0, shadowBuf.width, shadowBuf.height),
				Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
				&depthShader);
			m_evtMgr->emit<UnuseColorBufferEvent>(bufAliasname);
		}
		// Directional Light Pass
		for (auto obj : m_objMgr->entities<DirLightCom, DirLightTransform>()) {
			auto lightCommon = obj.component<LightCommon>();
			auto transCom = obj.component<DirLightTransform>();
			auto bufAliasname = "lightDepth" + std::to_string(obj.ID());
			if (colorBufferCom->dict.find(bufAliasname) == colorBufferCom->dict.end()) {
				continue;
			}
			ColorBufferRef& shadowBuf = colorBufferCom->dict[bufAliasname];
			m_evtMgr->emit<UseColorBufferEvent>(bufAliasname);
			

			auto shadowMapSettingCom = m_objMgr->getSingletonComponent<ShadowMapSetting>();
			auto shaderName = shadowMapSettingCom->shaderSetting[LightType::Dir][lightCommon->shadowType];
			Shader depthShader = getShader(shaderName);
			depthShader.use();
			depthShader.setMatrix4f("lightPV", transCom->lightPV);
			auto lightPV = transCom->lightPV;
			// dirLightShadowShader.set3f("lightPos", obj.component<SpatialData>()->pos);
			
			m_evtMgr->emit<RenderSceneEvent>(
				objCamera,
				std::make_tuple(0, 0, shadowBuf.width, shadowBuf.height),
				Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
				&depthShader);
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
		
		renderQuad();
		shader.set1i("begin", 0);
		for (auto obj : m_objMgr->entities<LightTag>()) {
			uploadLight(shader, obj);
			renderQuad();
		}
		glUseProgram(0);
		
		m_evtMgr->emit<UnuseColorBufferEvent>(colorBufferAliasName);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
    
	void RenderSystem::uploadLight(Shader shader, Object lightObject) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto lightCommon = lightObject.component<LightCommon>();
		shader.set1f("light.intensity", lightCommon->intensity);
		shader.set1i("light.shadowType", static_cast<int>(lightCommon->shadowType));
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
			shader.set3f("light.Position", spatialDataCom->pos);
			shader.setMatrix4f("light.lightPV", transCom->lightPV);
		} else if (lightObject.hasComponent<SpotLightCom>()) {
			auto spatialDataCom = lightObject.component<SpatialData>();
			auto lightCom = lightObject.component<SpotLightCom>();
			auto transCom = lightObject.component<SpotLightTransform>();
			shader.set1i("light.type", 3);
			shader.set3f("light.Direction", lightCom->direction);
			shader.set3f("light.Position", spatialDataCom->pos);
			shader.setMatrix4f("light.lightPV", transCom->lightPV);
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
				auto texRef = shadowBuf.depthTex;
				if (lightCommon->shadowType == ShadowType::VSM) {
					texRef = shadowBuf.tex;
				}
				m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "depthMap", 5, texRef);
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
			MeshRef, PointLightCom, SpatialData>()) {
			m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, lightShader);	
			m_evtMgr->emit<UploadMatrixToShaderEvent>(lightObj, lightShader);
			auto meshID = lightObj.component<MeshRef>()->meshID;
			m_evtMgr->emit<DrawMeshBufferEvent>(meshID, 0);
			
		}
		m_evtMgr->emit<UnuseColorBufferEvent>(colorBufferAliasName);
	}
   
	void RenderSystem::smaaPass(std::string inputBuffer, std::string outputBuffer) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& inputBuf = colorBufferCom->dict[inputBuffer];
		{
			// debug edge detect
			m_evtMgr->emit<UseColorBufferEvent>("edge");
			Shader edgeDetect = getShader("smaaEdgeDetect");
			edgeDetect.use();
			edgeDetect.set2f("imgSize", inputBuf.width, inputBuf.height);
			clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			m_evtMgr->emit<ActiveTextureByIDEvent>(edgeDetect, "colorTex", 0, inputBuf.tex);
			renderQuad();
			m_evtMgr->emit<UnuseColorBufferEvent>("edge");
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
			m_evtMgr->emit<UseColorBufferEvent>("weight");
			Shader smaaWeight = getShader("smaaWeight");
			smaaWeight.use();
			smaaWeight.set2f("imgSize", inputBuf.width, inputBuf.height);
			ColorBufferRef& edgeBuf = colorBufferCom->dict["edge"];
			clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			m_evtMgr->emit<ActiveTextureByIDEvent>(smaaWeight, "edgesTex", 0, edgeBuf.tex);
			m_evtMgr->emit<ActiveTextureByIDEvent>(smaaWeight, "areaTex", 1, area_tex);
			m_evtMgr->emit<ActiveTextureByIDEvent>(smaaWeight, "searchTex", 2, search_tex);
			renderQuad();
			m_evtMgr->emit<UnuseColorBufferEvent>("weight");
		}
		{
			m_evtMgr->emit<UseColorBufferEvent>(outputBuffer);
			Shader smaaBlending = getShader("smaaBlending");
			smaaBlending.use();
			smaaBlending.set2f("imgSize", inputBuf.width, inputBuf.height);
			ColorBufferRef& weightBuf = colorBufferCom->dict["weight"];
			ColorBufferRef& sceneBuf = colorBufferCom->dict[inputBuffer];
			m_evtMgr->emit<ActiveTextureByIDEvent>(smaaBlending, "colorTex", 0, sceneBuf.tex);
			m_evtMgr->emit<ActiveTextureByIDEvent>(smaaBlending, "blendTex", 1, weightBuf.tex);
			clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
				GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			renderQuad();
			m_evtMgr->emit<UnuseColorBufferEvent>(outputBuffer);
		}
	}

	void RenderSystem::renderTex(TexRef& tex, size_t winWidth, size_t winHeight, bool noGamma, bool noToneMapping) {
		Shader shader = getShader("screen");
		shader.use();
		shader.set1b("noGamma", noGamma);
		shader.set1b("noToneMapping", noToneMapping);
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "texture1", 0, tex);
		renderQuad();
	}

    void RenderSystem::renderColorBuffer(std::string colorBufferAliasName, size_t winWidth, size_t winHeight, bool noGamma, bool noToneMapping) {
        auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
        ColorBufferRef& buf = colorBufferCom->dict[colorBufferAliasName];
        renderTex(buf.tex, winWidth, winHeight, noGamma, noToneMapping);
    }

	void RenderSystem::renderDepthBuffer(std::string colorBufferAliasName, size_t winWidth, size_t winHeight, bool noGamma, bool noToneMapping) {
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& buf = colorBufferCom->dict[colorBufferAliasName];
		renderTex(buf.depthTex, winWidth, winHeight, noGamma, noToneMapping);
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
