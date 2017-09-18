#include "stdafx.h"
#include "realtime/glutils.hpp"
#include "system/renderSys.hpp"
#include "com/sdlContext.hpp"
#include "com/meshes.hpp"
#include "com/cameraCom.hpp"
#include "com/bufferCom.hpp"
#include "com/miscCom.hpp"
#include "system/spatialSys.hpp"
#include "event/materialEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/spatialEvent.hpp"
#include "event/bufferEvent.hpp"
#include "event/textureEvent.hpp"


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
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		/*----- first-pass: deferred rendering -----*/
		evtMgr.emit<UseGBufferEvent>("main");
		Shader gBufferShader = getShader("gBuffer");
		evtMgr.emit<RenderSceneEvent>(
			objMgr.getSingletonComponent<PerspectiveCameraView>().object(),
			std::make_tuple(0, 0, context->width, context->height),
			Color(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			&gBufferShader);
		evtMgr.emit<UnuseGBufferEvent>("main");
		
		/*----- first-pass end -----*/

        renderGBufferDebug("main", context->width, context->height);
        
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
		for (const Object obj : m_objMgr->entities<Meshes, MaterialCom, SpatialData, MeshBuffersCom>()) {
			auto matCom = obj.component<MaterialCom>();
			auto setting = matSetCom->settings[matCom->settingID];
			if (evt.shader == nullptr) {
				shader = getShader(setting);
				shader.use();
			}
			m_evtMgr->emit<ActiveMaterialEvent>(setting, shader);
			CheckGLError; 
			m_evtMgr->emit<ActiveSpatialDataEvent>(obj, shader);
			CheckGLError; 
			m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
			CheckGLError; 
			m_evtMgr->emit<UploadMatrixToShaderEvent>(obj, shader);
			CheckGLError; 
			m_evtMgr->emit<DrawMeshBufferEvent>(obj);
			CheckGLError;
			m_evtMgr->emit<DeactiveMaterialEvent>(setting);
			CheckGLError;
		}
		CheckGLError;
	}

	Shader RenderSystem::getShader(MaterialSettingCom& com) {
		auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
		return Shader(spSetCom->alias2HDL[com.shaderName]);
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
    
    void RenderSystem::renderGBufferDebug(std::string gBufferAliasName, size_t winWidth, size_t winHeight) {
        Shader shader = getShader("screen");
        shader.use();
        auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
        GBufferRef& buf = gBufferCom->dict[gBufferAliasName];
        
        clearView(Color(0.0f, 0.0f, 0.0f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        // left-top: position
        setViewport(std::make_tuple(0, winHeight / 2, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(0, buf.posTexID);
        renderQuad();
        
        // right-top: normal
        setViewport(std::make_tuple(winWidth / 2, winHeight / 2, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(0, buf.normalTexID);
        renderQuad();
        
        // left-bottom: albedo
        setViewport(std::make_tuple(0, 0, winWidth / 2, winHeight / 2));
        m_evtMgr->emit<ActiveTextureByIDEvent>(0, buf.albedoSpecTexID);
        renderQuad();
        
    }
};
