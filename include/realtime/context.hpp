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
		virtual void shutdown() {}
	};

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
};

#endif