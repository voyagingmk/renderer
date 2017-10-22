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
#include "event/uiEvent.hpp"
#include "utils/helper.hpp"


using namespace std;


namespace renderer {
    void RenderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("RenderSystem init\n");
		evtMgr.on<RenderSceneEvent>(*this);	
    }
    
	// renderpipe loop, could move to another system
	void RenderSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		auto context = objMgr.getSingletonComponent<SDLContext>();
		if (!context.valid()) {
			return;
		}
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		/*----- first-pass: deferred rendering-----*/
		
		Object objCamera = objMgr.getSingletonComponent<PerspectiveCameraView>().object();
		//  geometry pass
		evtMgr.emit<UseGBufferEvent>("main");
		Shader gBufferShader = getShader("gBuffer");
		evtMgr.emit<RenderSceneEvent>(
			objCamera,
			std::make_tuple(0, 0, context->width, context->height),
			Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			&gBufferShader);
		evtMgr.emit<UnuseGBufferEvent>("main");
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// ssao pass
        ssaoPass(objCamera, "main", "ssao", context->width, context->height);
		ssaoBlurPass(objCamera, "ssao", "ssaoBlur", context->width, context->height);

		// shadow cubemap pass
		evtMgr.emit<UseColorBufferEvent>("shadow");
		CheckGLError; 
		Shader pointShadowDepthShader = getShader("pointShadowDepth");
		pointShadowDepthShader.use();
		auto pointLightTrans = m_objMgr->getSingletonComponent<PointLightTransform>();
		pointShadowDepthShader.setMatrixes4f("lightPVs", pointLightTrans->lightPVs);
		pointShadowDepthShader.set1f("far_plane", pointLightTrans->f);
        pointShadowDepthShader.set3f("lightPos", pointLightTrans.object().component<SpatialData>()->pos);
        CheckGLError; auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& buf = colorBufferCom->dict["shadow"];
		evtMgr.emit<RenderSceneEvent>(
			objCamera,
			std::make_tuple(0, 0, buf.width, buf.height),
			Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			&pointShadowDepthShader);
		evtMgr.emit<UnuseColorBufferEvent>("shadow");
        CheckGLError;
        
		// lighting pass
        deferredLightingPass(objCamera, "main", context->width, context->height);
        CheckGLError;
        
		evtMgr.emit<CopyGBufferDepthEvent>("main");// skybox 需要深度信息
        // skybox pass
        renderSkybox(objCamera);
        
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
		 //setViewport(std::make_tuple(0, 0, context->width, context->height));
		// clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
		// GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
		/*----- first-pass end -----*/
        // renderGBufferDebug("main", context->width, context->height);
        // renderColorBufferDebug("ssaoBlur", context->width, context->height);

		// debug edge detect
		Shader edegeDetect = getShader("edegeDetect");
		edegeDetect.use();
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& gBuf = gBufferCom->dict["main"];
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		setViewport(std::make_tuple(0, 0, context->width, context->height));
		m_evtMgr->emit<ActiveTextureByIDEvent>(edegeDetect, "colorTex", 0, gBuf.albedoTexID);
		renderQuad();

        // m_evtMgr->emit<DrawUIEvent>();
		CheckGLError;
		SDL_GL_SwapWindow(context->win);
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
			auto setting = matSetCom->settings[matCom->settingID];
			if (evt.shader == nullptr) {
				shader = getShader(setting);
				shader.use();
                uploadLights(shader);
			}
			m_evtMgr->emit<ActiveMaterialEvent>(matCom->settingID, shader);
			CheckGLError; 
			m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
			CheckGLError; 
			m_evtMgr->emit<UploadMatrixToShaderEvent>(obj, shader);
			CheckGLError; 
			m_evtMgr->emit<DrawMeshBufferEvent>(obj);
			CheckGLError;
			m_evtMgr->emit<DeactiveMaterialEvent>(matCom->settingID);
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
    
	void RenderSystem::renderSkybox(Object objCamera) {
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

	void RenderSystem::deferredLightingPass(Object objCamera, std::string gBufferAliasName, size_t winWidth, size_t winHeight) {
		Shader shader = getShader("deferredShading");
		shader.use();
		m_evtMgr->emit<UploadCameraToShaderEvent>(objCamera, shader);
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& buf = gBufferCom->dict[gBufferAliasName];
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gPosition", 0, buf.posTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gNormal", 1, buf.normalTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gAlbedo", 2, buf.albedoTexID);
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "gPBR", 3, buf.pbrTexID);
		auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& ssaoBuf = colorBufferCom->dict["ssaoBlur"];
		m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "ssao", 4, ssaoBuf.tex.texID);
        ColorBufferRef& shadowBuf = colorBufferCom->dict["shadow"];
        m_evtMgr->emit<ActiveTextureByIDEvent>(shader, "depthMap", 5, shadowBuf.depthTex);
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        uploadLights(shader);
		renderQuad();
	}
    
    void RenderSystem::uploadLights(Shader shader) {
        uint32_t i = 0;
        for (auto obj : m_objMgr->entities<PointLightCom, SpatialData>()) {
            auto spatialDataCom = obj.component<SpatialData>();
            auto transCom = obj.component<PointLightTransform>();
            auto lightCom = obj.component<PointLightCom>();
            shader.set3f(("lights[" + std::to_string(i) + "].Position").c_str(), spatialDataCom->pos);
            shader.set3f(("lights[" + std::to_string(i) + "].Color").c_str(), lightCom->ambient);
            shader.set1f(("lights[" + std::to_string(i) + "].far_plane").c_str(), transCom->f);
            shader.set1f(("lights[" + std::to_string(i) + "].Linear").c_str(), lightCom->linear);
            shader.set1f(("lights[" + std::to_string(i) + "].Quadratic").c_str(), lightCom->quadratic);
            shader.set1f(("lights[" + std::to_string(i) + "].constant").c_str(), lightCom->constant);
            i++;
        }
        shader.set1i("LightNum", i);
    }
    
    
    void RenderSystem::renderColorBufferDebug(std::string colorBufferAliasName, size_t winWidth, size_t winHeight) {
        Shader shader = getShader("screen");
        shader.use();
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
