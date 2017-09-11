#include "stdafx.h"
#include "system/render.hpp"
#include "com/sdlContext.hpp"
#include "com/meshes.hpp"

using namespace std;

namespace renderer {
    void RenderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("RenderSystem init\n");
    }
    
	// renderpipe loop
    void RenderSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		auto context = objMgr.getSingltonComponent<SDLContext>();
		if (!context.valid()) {
			return;
		}
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); 
		glViewport(0, 0, context->width, context->height);
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		for(auto meshes: objMgr.entities<Meshes>()) {
            
        }
		SDL_GL_SwapWindow(context->win);
	}
};
