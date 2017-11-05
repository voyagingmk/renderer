#include "stdafx.h"
#include "system/envSys.hpp"

using namespace std;

namespace renderer {
	void EnvSystem::init(ObjectManager &objMgr, EventManager &evtMgr)
	{
		printf("EnvSystem init\n");
		evtMgr.on<SetupSDLEvent>(*this);
		evtMgr.on<ComponentAddedEvent<SDLContext>>(*this);
		evtMgr.on<ComponentRemovedEvent<SDLContext>>(*this);
		evtMgr.on<CustomSDLKeyboardEvent>(*this);
	}

	void EnvSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
		auto com = objMgr.getSingletonComponent<SDLContext>();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
				evtMgr.emit<CustomSDLKeyboardEvent>(e.key);
			}
			else if (e.type == SDL_MOUSEMOTION) {
				if (!com->showCursor) {
					evtMgr.emit<CustomSDLMouseMotionEvent>(e.motion);
				}
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN ||
				e.type == SDL_MOUSEBUTTONUP ||
				e.type == SDL_MOUSEWHEEL) {
				evtMgr.emit<CustomSDLMouseButtonEvent>(e.button);
			}
			else {
				evtMgr.emit<CustomSDLEvent>(e);
			}
			if (com->shouldExit) {
				exit(0);
				break;
			}
		}
	}

	void EnvSystem::receive(const SetupSDLEvent &evt) {
		Object obj = m_objMgr->create(); // singleTon, manage kinds of resources
		obj.addComponent<SDLContext>(evt.winWidth, evt.winHeight);
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
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
			SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);
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

		//SDL_ShowCursor(SDL_DISABLE);
		//SDL_SetWindowGrab(sdlContext->win, SDL_TRUE);
		// SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
		SDL_SetRelativeMouseMode(SDL_TRUE);
        
        m_evtMgr->emit<SetupSDLDoneEvent>(obj);
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
		auto context = m_objMgr->getSingletonComponent<SDLContext>();
		auto renderMode = m_objMgr->getSingletonComponent<RenderMode>();
		auto keyState = m_objMgr->getSingletonComponent<KeyState>();

		auto k = evt.e.keysym.sym;
		keyState->state[k] = evt.e.state;
		// printf("key event: %d, %d\n", k, evt.e.state);

		if (evt.e.state == SDL_PRESSED)
		{
			switch (k)
			{
			case SDLK_ESCAPE:
			{
				context->shouldExit = true;
				break;
			}
			case SDLK_TAB:
			{
				context->showCursor = !context->showCursor;
				SDL_ShowCursor(context->showCursor? SDL_ENABLE : SDL_DISABLE);
				SDL_SetRelativeMouseMode(context->showCursor ? SDL_FALSE : SDL_TRUE);
				printf("ShowCursor: %d\n", int(context->showCursor));
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

	void EnvSystem::receive(const CustomSDLEvent &evt)
	{
		auto com = m_objMgr->getSingletonComponent<SDLContext>();
		switch (evt.e.type)
		{
		case SDL_QUIT:
			com->shouldExit = true;
			break;
		default:
			break;
		}
	}

	void EnvSystem::shutdown(const char *msg) {
		printf("%s, SDLErr: %s\n", msg ? msg : "no msg", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
}
