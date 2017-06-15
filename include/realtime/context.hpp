#ifndef RENDERER_REALTIME_CONTEXT_HPP
#define RENDERER_REALTIME_CONTEXT_HPP

#include "base.hpp"

namespace renderer {

	class RendererContextBase {
	public:
		size_t winWidth, winHeight;
	public:
        RendererContextBase(): winWidth(0), winHeight(0) {}
		virtual void setTitle(const char* title) {}
		virtual void setup(size_t w, size_t h) {}
        virtual void onCustomSetup() {}
        virtual void loop() {}
		virtual void onPoll() {}
		virtual void resize(int w, int h) {}
		virtual void shutdown(const char *msg = nullptr) {}
        virtual void onCustomDestroy() {}
	};



	class RendererContextSDL: public RendererContextBase {
	protected:
		bool shouldExit;
	public:
		SDL_Window * win;
		SDL_Renderer * rendererSDL;
		SDL_GLContext glContext;
	public:
        RendererContextSDL():
            shouldExit(false),
            win(nullptr),
            rendererSDL(nullptr),
            glContext(nullptr) {}
		virtual void setTitle(const char* title) override;
		virtual void setup(size_t w, size_t h) override;
		virtual void loop() override;
		virtual void onPoll() override;
		virtual void shutdown(const char *msg = nullptr) override;
		virtual void resize(int w, int h) override;
		virtual void onSDLEvent(SDL_Event& e) {}
        virtual void onSDLMouseEvent(SDL_MouseMotionEvent& e) {}
        virtual void onSDLMouseEvent(SDL_MouseButtonEvent& e) {}
        virtual void onSDLKeyboardEvent(SDL_KeyboardEvent& e) {}
	};




#ifdef USE_GLFW
	class RendererContextOpenGL: public RendererContextBase {
		static std::map<GLFWwindow*, RendererContextOpenGL*> win2ContextPtr;
	public:
		GLFWwindow* window = nullptr;
	public:
		void setTitle(const char* title) override;
		virtual void setup(size_t w, size_t h) override;
		virtual void loop() override;
		virtual void onPoll() {};
		virtual void shutdown();
	public:
		void inspecetGPU();
	public:
		// static
		static RendererContextOpenGL* getContext(GLFWwindow* w) {
			return win2ContextPtr[w];
		}
	protected:
		virtual void onKeyEvent(int key, int scancode, int action, int mode) {}
	};
#endif

};
#endif
