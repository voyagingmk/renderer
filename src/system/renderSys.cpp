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

		evtMgr.emit<UseGBufferEvent>("main");

		Shader gBufferShader = getShader("gBuffer");
		auto com = objMgr.getSingletonComponent<PerspectiveCameraView>();
		evtMgr.emit<RenderSceneEvent>(
			com.object(),
			std::make_tuple(0, 0, context->width, context->height),
			Color(0.5f, 0.5f, 0.5f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			&gBufferShader);
		evtMgr.emit<UnuseGBufferEvent>("main");
		
		Shader shader = getShader("screen");
		shader.use();
		auto gBufferCom = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef& buf = gBufferCom->dict["main"];
        evtMgr.emit<ActiveTextureByIDEvent>(0, buf.normalTexID);
		
		renderQuad(context->width, context->height);

		CheckGLError;
		SDL_GL_SwapWindow(context->win);
	}
    
    void RenderSystem::resetView(const Viewport& viewport, const Color clearColor, const uint32_t clearBits) {
        glViewport(std::get<0>(viewport),
                   std::get<1>(viewport),
                   std::get<2>(viewport),
                   std::get<3>(viewport));
        glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
        glClear(clearBits);
    }
    
	void RenderSystem::receive(const RenderSceneEvent &evt) {
		ObjectManager& objMgr = evt.objCamera.objMgr();
		EventManager& evtMgr = objMgr.evtMgr();
		auto matSetCom = objMgr.getSingletonComponent<MaterialSet>();
		// ShaderMgr &shaderMgr = ShaderMgr::getInstance();
		// Shader &shader = shaderMgr.getShader(shaderALias);
		// shader.set3f("viewPos", viewPos);
		// shader.setMatrix4f("PV", PV);
		glEnable(GL_DEPTH_TEST);
        resetView(evt.viewport, evt.clearColor, evt.clearBits);
		
		Shader shader;

		if (evt.shader != nullptr) {
			shader = *evt.shader;
			shader.use();
		}

		// TODO: sort by material
		for (const Object obj : objMgr.entities<Meshes, MaterialCom, SpatialData, MeshBuffersCom>()) {
			auto matCom = obj.component<MaterialCom>();
			auto setting = matSetCom->settings[matCom->settingID];
			if (evt.shader == nullptr) {
				shader = getShader(setting);
				shader.use();
			}
			evtMgr.emit<ActiveMaterialEvent>(setting, shader);
			CheckGLError; 
			evtMgr.emit<ActiveSpatialDataEvent>(obj, shader);
			CheckGLError; 
			evtMgr.emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
			CheckGLError; 
			evtMgr.emit<UploadMatrixToShaderEvent>(obj, shader);
			CheckGLError; 
			evtMgr.emit<DrawMeshBufferEvent>(obj);
			CheckGLError;
			evtMgr.emit<DeactiveMaterialEvent>(setting);
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

    void RenderSystem::renderQuad(size_t winWidth, size_t winHeight) {
        resetView(std::make_tuple(0, 0, winWidth, winHeight),
            Color(0.0f, 0.0f, 0.0f, 1.0f),
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        for (auto obj : m_objMgr->entities<GlobalQuadTag>()) {
			m_evtMgr->emit<DrawMeshBufferEvent>(obj);
		}
	}
};
