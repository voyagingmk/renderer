#include "stdafx.h"
#include "main_raytrace.hpp"
#include "realtime/shadermgr.hpp"
#include "realtime/context.hpp"



using namespace std;
using namespace renderer;


class MyContext : public RendererContextSDL {
    ShaderMgrOpenGL shaderMgr;
    ShaderProgramHDL shaderHDL;
public:
	virtual void onSDLEvent(SDL_Event& e) override {
		if (e.type == SDL_QUIT) {
			shouldExit = true;
		}
	}
    virtual void onCustomSetup() override {
        shaderMgr.setShaderFileDirPath("assets/shaders/");
       shaderHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "test1.vs" },
            { ShaderType::Fragment, "test1.fs"}
        });
        GLfloat vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        }; 
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }
    virtual void onPoll() override
    {
        
    }
};


int main(int argc, char *argv[]) {
    // rayTraceMain(argc, argv);
    // return 0;
    printf("aa");
	MyContext context;
	context.setup(800, 600);
	context.loop();
	return 0;
}
