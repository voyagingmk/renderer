#include "stdafx.h"
#include "system/env.hpp"

using namespace std;

namespace renderer {
	void EnvSystem::init(ObjectManager &objMgr, EventManager &evtMgr)
	{
		evtMgr.on<ComponentAddedEvent<SDLContext>>(*this);
		evtMgr.on<ComponentRemovedEvent<SDLContext>>(*this);

		evtMgr.on<CustomSDLEvent>(*this);
		evtMgr.on<CustomSDLKeyboardEvent>(*this);
		evtMgr.on<CustomSDLMouseMotionEvent>(*this);
		evtMgr.on<CustomSDLMouseButtonEvent>(*this);


		Object obj = objMgr.create();
		obj.addComponent<SDLContext>(800, 600);
		obj.addComponent<RenderMode>();
		obj.addComponent<KeyState>();
	}

	void EnvSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		for (auto obj : objMgr.entities<SDLContext>()) {
			auto com = obj.component<SDLContext>();
			while (1) {
				SDL_Event e;
				if (SDL_PollEvent(&e)) {
					if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
						evtMgr.emit<CustomSDLKeyboardEvent>(obj, e.key);
					}
					else if (e.type == SDL_MOUSEMOTION) {
						evtMgr.emit<CustomSDLMouseMotionEvent>(obj, e.motion);
					}
					else if (e.type == SDL_MOUSEBUTTONDOWN ||
						e.type == SDL_MOUSEBUTTONUP ||
						e.type == SDL_MOUSEWHEEL) {
						evtMgr.emit<CustomSDLMouseButtonEvent>(obj, e.button);
					}
					else {
						evtMgr.emit<CustomSDLEvent>(obj, e);
					}
				}
				if (com->shouldExit) {
					break;
				}
				// onPoll();
				SDL_GL_SwapWindow(com->win);
				// SDL_RenderPresent(rendererSDL);
			}
			break;
		}
	}

	void EnvSystem::receive(const ComponentAddedEvent<SDLContext> &evt) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			shutdown("Unable to initialize SDL");
			return;
		}
		checkSDLError(__LINE__);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		checkSDLError(__LINE__);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		checkSDLError(__LINE__);//on Win7 would cause a ERROR about SHCore.dll, just ignore it.
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		checkSDLError(__LINE__);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		checkSDLError(__LINE__);

		Object obj = evt.m_obj;
		auto sdlContext = obj.component<SDLContext>();
		sdlContext->win = SDL_CreateWindow("",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			sdlContext->width,
			sdlContext->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!sdlContext->win) {
			shutdown("Unable to create window");
			return;
		}
		sdlContext->glContext = SDL_GL_CreateContext(sdlContext->win);

		if (gl3wInit() == -1) {
			shutdown("gl3wInit failed");
			return;
		}
		if (!gl3wIsSupported(3, 2)) {
			shutdown("OpenGL 3.2 not supported");
			return;
		}
		checkSDLError(__LINE__);
		const char* ogl_version = (const char*)glGetString(GL_VERSION);
		const char* glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		if (ogl_version) {
			cout << "OpenGL version " << ogl_version << endl;
		}
		else {
			cout << "OpenGL version error " << endl;
		}
		if (glsl_version) {
			cout << "GLSL version " << glsl_version << endl;
		}
		else {
			cout << "GLSL version error " << endl;
		}
		GLint nrAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		cout << "Maximum nr of vertex attributes supported: " << nrAttributes << endl;
	}

	void EnvSystem::receive(const ComponentRemovedEvent<SDLContext> &evt) {
		ComponentHandle<SDLContext> com = evt.component;
		if (com->rendererSDL) {
			SDL_DestroyRenderer(com->rendererSDL);
		}
		if (com->glContext) {
			SDL_GL_DeleteContext(com->glContext);
		}
		if (com->win) {
			SDL_DestroyWindow(com->win);
		}
	}

	void EnvSystem::receive(const CustomSDLKeyboardEvent &evt)
	{
		auto context = evt.obj.component<SDLContext>();
		auto renderMode = evt.obj.component<RenderMode>();
		auto keyState = evt.obj.component<KeyState>();

		auto k = evt.e.keysym.sym;
		keyState->state[k] = evt.e.state;

		if (evt.e.state == SDL_PRESSED)
		{
			switch (k)
			{
			case SDLK_ESCAPE:
			{
				context->shouldExit = true;
				break;
			}
			case SDLK_1:
			{
				// normal
				renderMode->mode = RenderModeEnum::Normal;
				break;
			}
			case SDLK_2:
			{
				// depth map
				renderMode->mode = RenderModeEnum::DepthMap;
				break;
			}
			default:
				break;
			}
		}
	}

	void EnvSystem::shutdown(const char *msg) {
		printf("%s, SDLErr: %s\n", msg ? msg : "no msg", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
}
