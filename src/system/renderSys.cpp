#include "stdafx.h"
#include "realtime/glutils.hpp"
#include "system/renderSys.hpp"
#include "com/sdlContext.hpp"
#include "com/meshes.hpp"
#include "com/materialCom.hpp"
#include "com/cameraCom.hpp"
#include "system/spatialSys.hpp"


using namespace std;

namespace renderer {
    void RenderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("RenderSystem init\n");
		evtMgr.on<RenderSceneEvent>(*this);	
		Object objCamera = objMgr.create();
		objCamera.addComponent<PerspectiveCameraView>();
    }
    
	// renderpipe loop, could move to another system
	void RenderSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		auto context = objMgr.getSingletonComponent<SDLContext>();
		if (!context.valid()) {
			return;
		}
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glViewport(0, 0, context->width, context->height);

		auto com = objMgr.getSingletonComponent<PerspectiveCameraView>();
		evtMgr.emit<RenderSceneEvent>(
			com.object(),
			std::make_tuple(0, 0, context->width, context->height));

		SDL_GL_SwapWindow(context->win);
	}

	void RenderSystem::receive(const RenderSceneEvent &evt) {
		ObjectManager& objMgr = evt.objCamera.objMgr();

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
		// shader.setLight(light);
		// drawTerrian(shader);
		// drawObjs(shader);
		for (const Object objs : objMgr.entities<Meshes, MaterialCom, SpatialData>()) {

		}
		CheckGLError;
	}
};
