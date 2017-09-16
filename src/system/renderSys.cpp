#include "stdafx.h"
#include "realtime/glutils.hpp"
#include "system/renderSys.hpp"
#include "com/sdlContext.hpp"
#include "com/meshes.hpp"
#include "com/materialCom.hpp"
#include "com/cameraCom.hpp"
#include "com/bufferCom.hpp"
#include "system/spatialSys.hpp"
#include "event/materialEvent.hpp"
#include "event/shaderEvent.hpp"
#include "event/spatialEvent.hpp"
#include "event/bufferEvent.hpp"


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

		auto com = objMgr.getSingletonComponent<PerspectiveCameraView>();
		evtMgr.emit<RenderSceneEvent>(
			com.object(),
			std::make_tuple(0, 0, context->width, context->height));

		SDL_GL_SwapWindow(context->win);
	}

	void RenderSystem::receive(const RenderSceneEvent &evt) {
		ObjectManager& objMgr = evt.objCamera.objMgr();
		EventManager& evtMgr = objMgr.evtMgr();
		auto matSetCom = objMgr.getSingletonComponent<MaterialSet>();
		auto spSeCom = objMgr.getSingletonComponent<ShaderProgramSet>();
		// ShaderMgr &shaderMgr = ShaderMgr::getInstance();
		// Shader &shader = shaderMgr.getShader(shaderALias);
		// shader.set3f("viewPos", viewPos);
		// shader.setMatrix4f("PV", PV);
		glEnable(GL_DEPTH_TEST);
		glViewport(std::get<0>(evt.viewport),
			std::get<1>(evt.viewport), 
			std::get<2>(evt.viewport),
			std::get<3>(evt.viewport));
		glClearColor(evt.clearColor.r(), evt.clearColor.g(), evt.clearColor.b(), evt.clearColor.a());
		glClear(evt.clearBits);


		// TODO: sort by material
		for (const Object obj : objMgr.entities<Meshes, MaterialCom, SpatialData, BufferSetsCom>()) {
			auto matCom = obj.component<MaterialCom>();
			auto setting = matSetCom->settings[matCom->settingID];
			Shader shader(spSeCom->alias2HDL[setting.shaderName]);
			shader.use();
			evtMgr.emit<ActiveMaterialEvent>(setting, shader);
			evtMgr.emit<ActiveSpatialDataEvent>(obj, shader);
			evtMgr.emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
			evtMgr.emit<UploadMatrixToShaderEvent>(obj, shader);
			evtMgr.emit<DrawBufferEvent>(obj);
			evtMgr.emit<DeactiveMaterialEvent>(setting);
			

		}
		CheckGLError;
	}
};
