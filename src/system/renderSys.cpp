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

		// shadow cubemap pass
		evtMgr.emit<UseColorBufferEvent>("shadow");
		CheckGLError; 
		Shader pointShadowDepthShader = getShader("pointShadowDepth");
		pointShadowDepthShader.use();
		auto pointLightTrans = m_objMgr->getSingletonComponent<PointLightTransform>();
		pointShadowDepthShader.setMatrixes4f("lightPVs", pointLightTrans->lightPVs);
        float far_plane = pointLightTrans->f;
		pointShadowDepthShader.set1f("far_plane", far_plane);
        pointShadowDepthShader.set3f("lightPos", pointLightTrans.object().component<SpatialData>()->pos);
		pointShadowDepthShader.set1f("normalOffset", gSettingCom->get1f("normalOffset", -1.3f));
        CheckGLError;
        auto colorBufferCom = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef& buf = colorBufferCom->dict["shadow"];
		glCullFace(GL_FRONT);
		evtMgr.emit<RenderSceneEvent>(
			objCamera,
			std::make_tuple(0, 0, buf.width, buf.height),
			Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			&pointShadowDepthShader);
		evtMgr.emit<UnuseColorBufferEvent>("shadow");
		glCullFace(GL_BACK);
        CheckGLError;
        
		// lighting pass
        deferredLightingPass("core", objCamera, "main", context->width, context->height);
        CheckGLError;

		 //setViewport(std::make_tuple(0, 0, context->width, context->height));
		// clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
		// GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
		/*----- first-pass end -----*/

        ColorBufferRef& coreBuf = colorBufferCom->dict["core"];
        
		// debug edge detect
        {
            evtMgr.emit<UseColorBufferEvent>("edge");
            Shader edgeDetect = getShader("smaaEdgeDetect");
            edgeDetect.use();
            edgeDetect.set2f("imgSize", context->width, context->height);
            clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            setViewport(screenViewport);
            m_evtMgr->emit<ActiveTextureByIDEvent>(edgeDetect, "colorTex", 0, coreBuf.tex);
            renderQuad();
            evtMgr.emit<UnuseColorBufferEvent>("edge");
        }
        
        static unsigned int area_tex = 0;
        static unsigned int search_tex = 0;
        if (area_tex == 0) {
            glGenTextures(1, &area_tex);
            glBindTexture( GL_TEXTURE_2D, area_tex );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RG8, ( GLsizei )AREATEX_WIDTH, ( GLsizei )AREATEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, areaTexBytes );
        }
        if (search_tex == 0) {
            glGenTextures( 1, &search_tex );
            glBindTexture( GL_TEXTURE_2D, search_tex );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, ( GLsizei )SEARCHTEX_WIDTH, ( GLsizei )SEARCHTEX_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, searchTexBytes );
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
            Shader smaa = getShader("smaaBlending");
            smaa.use();
            smaa.set2f("imgSize", context->width, context->height);
            ColorBufferRef& weightBuf = colorBufferCom->dict["weight"];
            m_evtMgr->emit<ActiveTextureByIDEvent>(smaa, "colorTex", 0, coreBuf.tex);
            m_evtMgr->emit<ActiveTextureByIDEvent>(smaa, "blendTex", 1, weightBuf.tex);
            clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            setViewport(screenViewport);
            renderQuad();
        }
		
		// skybox pass
		evtMgr.emit<CopyGBufferDepth2ColorBufferEvent>("main", "");// 画skybox需要GBuffer的深度信息
		CheckGLError;
		renderSkybox("", objCamera, screenViewport);
		renderLightObjects("", objCamera, screenViewport);

		// renderColorBufferDebug("core", context->width, context->height);
		// renderGBufferDebug("main", context->width, context->height);
		// renderColorBufferDebug("ssaoBlur", context->width, context->height);
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
			m_evtMgr->emit<ActiveTextureEvent>(shader, "normalMap", 0, "GA_CW_N_1");
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
					uploadLights(shader);
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

	void RenderSystem::deferredLightingPass(std::string colorBufferAliasName, Object objCamera, std::string gBufferAliasName, size_t winWidth, size_t winHeight) {
		m_evtMgr->emit<UseColorBufferEvent>(colorBufferAliasName);
		auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
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
		shader.set1f("depthBias", gSettingCom->get1f("depthBias", 1.0f));
        shader.set1i("enableSSAO", gSettingCom->get1b("enableSSAO", true));
		shader.set1f("diskFactor", gSettingCom->get1f("diskFactor", 3.0f));
		setViewport(std::make_tuple(0, 0, winWidth, winHeight));
		clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        uploadLights(shader);
		renderQuad();
		m_evtMgr->emit<UnuseColorBufferEvent>(colorBufferAliasName);
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
			auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
			float constant = gSettingCom->get1f("pointLightConstant");
			float linear = gSettingCom->get1f("pointLightLinear");
			float quadratic = gSettingCom->get1f("pointLightQuad");
			shader.set1f(("lights[" + std::to_string(i) + "].constant").c_str(), constant);
			shader.set1f(("lights[" + std::to_string(i) + "].Linear").c_str(), linear);
			shader.set1f(("lights[" + std::to_string(i) + "].Quadratic").c_str(), quadratic);
			//shader.set1f(("lights[" + std::to_string(i) + "].constant").c_str(), lightCom->constant);
			//shader.set1f(("lights[" + std::to_string(i) + "].Linear").c_str(), lightCom->linear);
            //shader.set1f(("lights[" + std::to_string(i) + "].Quadratic").c_str(), lightCom->quadratic);
            i++;
        }
        shader.set1i("LightNum", i);
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
