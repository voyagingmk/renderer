#include "stdafx.h"
#include "realtime/context.hpp"

using namespace renderer;
using namespace std;
using namespace placeholders;

void RendererContextOpenGL::setTitle(const char* title) {
	glfwSetWindowTitle(window, title);
}

void RendererContextOpenGL::setup(size_t w, size_t h) {
	winWidth = w;
	winHeight = h;

	if (!glfwInit()) {
		cout << "err: could not start GLFW3" << endl;
		return;
	}
	int major, minor, revision;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwGetVersion(&major, &minor, &revision);
	cout << "major: " << major << " minor: " << minor << " revision: " << revision << endl;
	cout << glfwGetVersionString() << endl;
	window = glfwCreateWindow(winWidth, winHeight, "", nullptr, nullptr);
	if (!window) {
		cout << "err: could not open window with GLFW3" << endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	cout << "renderer: %s" << renderer << endl;
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	cout << "OpenGL version supported %s" << version << endl;

	/*
		init callbacks
	*/
	// Set the required callback functions
	glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mode) {
		RendererContextOpenGL::getContext(win)->onKeyEvent(key, scancode, action, mode);
	});
	glfwSetErrorCallback([](int code, const char* info) {
		cout << "glfw err: " << code << ", info: " << info << endl;
	});

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	win2ContextPtr[window] = this;
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	// glEnable(GL_DEPTH_TEST); // enable depth-testing
	// glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glViewport(0, 0, winWidth, winHeight);

	/*
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	cout << "glfwGetFramebufferSize: " << width << " , " << height << endl;
	*/

}

void RendererContextOpenGL::loop() {
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		onPoll();
	}
}


void RendererContextOpenGL::inspecetGPU() {
	#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
	#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
	GLint nTotalMemoryInKB = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &nTotalMemoryInKB);

	GLint nCurAvailMemoryInKB = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &nCurAvailMemoryInKB);
	
	cout << "total: " << nTotalMemoryInKB << " avail: " << nCurAvailMemoryInKB << endl;
}


void RendererContextOpenGL::shutdown() { 
	if (window) {
		glfwDestroyWindow(window);
		window = nullptr;
	}
}

std::map<GLFWwindow*, RendererContextOpenGL*> RendererContextOpenGL::win2ContextPtr;