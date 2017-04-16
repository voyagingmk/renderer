#ifndef RENDERER_REALTIME_CONTEXT_HPP
#define RENDERER_REALTIME_CONTEXT_HPP

#include "base.hpp"

namespace renderer {

	class RendererContextBase {
	public:
		size_t winWidth, winHeight;
	public:
		virtual void setTitle(const char* title) {}
		virtual void setup(size_t w, size_t h) {}
		virtual void loop() {}
		virtual void onPoll() {}
		virtual void resize(int w, int h) {}
		virtual void shutdown(const char *msg = nullptr) {}
	};



	class RendererContextSDL: public RendererContextBase {
	protected:
		bool shouldExit = false;
	public:
		SDL_Window * win = nullptr;
		SDL_Renderer * rendererSDL = nullptr;
		SDL_GLContext glContext = nullptr;
	public:
		virtual void setTitle(const char* title) override;
		virtual void setup(size_t w, size_t h) override;
		virtual void loop() override;
		virtual void onPoll() override;
		virtual void shutdown(const char *msg = nullptr) override;
		virtual void resize(int w, int h) override;
	public:
		virtual void onSDLEvent(SDL_Event&) {}
	};




#ifdef USE_GLEW
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