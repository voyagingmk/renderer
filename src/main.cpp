#include "stdafx.h"
#include "main_raytrace.hpp"
#include "realtime/shadermgr.hpp"
#include "realtime/texturemgr.hpp"
#include "realtime/context.hpp"
#include "transform.hpp"
#include "geometry.cpp"
#include "quaternion.hpp"



using namespace std;
using namespace renderer;


class MyContext : public RendererContextSDL {
    ShaderProgramHDL shaderProgramHDL;
    GLuint VBO, VAO, EBO;
    texID texID1, texID2;
public:
    MyContext():
        shaderProgramHDL(0),
        VBO(0),
        VAO(0),
        EBO(0),
        texID1(0), texID2(0) {}
	virtual void onSDLEvent(SDL_Event& e) override {
		if (e.type == SDL_QUIT) {
			shouldExit = true;
		}
	}
    virtual void onCustomSetup() override {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        texMgr.setTextureDirPath("assets/images/");
        shaderMgr.setShaderFileDirPath("assets/shaders/");
        shaderProgramHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "test1.vs" },
            { ShaderType::Fragment, "test1.fs"}
        });
        texID1 = texMgr.loadTexture("container.jpg", "container");
        texID2 = texMgr.loadTexture("face.png", "face");
        // Set up vertex data (and buffer(s)) and attribute pointers
        GLfloat vertices[] = {
            // Positions          // Colors           // Texture Coords
            0.5f,  0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
            0.5f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
            -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left
        };
        GLuint indices[] = {  // Note that we start from 0!
            0, 1, 3, // First Triangle
            1, 2, 3  // Second Triangle
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        // TexCoord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
        
        glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
        
        
        // Uncommenting this call will result in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glViewport(0, 0, winWidth, winHeight);
        // Setup OpenGL options
        glEnable(GL_DEPTH_TEST);
        
    }
    virtual void onPoll() override
    {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        shaderMgr.useShaderProgram(shaderProgramHDL);
        
        // Bind Textures using texture units
        texMgr.activateTexture(0, texID1);
        UniLoc loc1 = shaderMgr.getUniformLocation(shaderProgramHDL, "ourTexture1");
        shaderMgr.setUniform1i(loc1, 0);

        texMgr.activateTexture(1, texID2);
        UniLoc loc2 = shaderMgr.getUniformLocation(shaderProgramHDL, "ourTexture2");
        shaderMgr.setUniform1i(loc2, 1);
        
        UniLoc locModel = shaderMgr.getUniformLocation(shaderProgramHDL, "model");
        UniLoc locView = shaderMgr.getUniformLocation(shaderProgramHDL, "view");
        UniLoc locProj = shaderMgr.getUniformLocation(shaderProgramHDL, "proj");
        float x = sin(getTimeMS()*0.001) * 0.5f;
        Transform4x4 trans1 = Translate(Vector3dF(0.0, 0.0, -2.0));
        Transform4x4 trans2 = Scale(0.5, 0.5, 0.5);
        static float pitch = 0.0, yaw = 0.0, roll = 0.0;
        pitch += 4.0f;
        yaw += 2.0f;
        roll += 3.0f;
        QuaternionF rotX = QuaternionF::RotateX(pitch);
        QuaternionF rotY = QuaternionF::RotateY(pitch);
        QuaternionF rotZ = QuaternionF::RotateZ(pitch);
        QuaternionF rot = rotX * rotY * rotZ;
        Transform4x4 modelTrans = trans1 * trans2 * Transform4x4(rot.toMatrix4x4());
        Transform4x4 projTrans = Perspective(45.0, winWidth / (float)winHeight, 0.1, 100.0);

        Transform4x4 viewTrans = LookAt(Vector3dF(0.0, 0.0, 1.0), Vector3dF(0.0, 0.0, -1.0), Vector3dF(0.0,1.0,0.0));
        // Matrix4x4::newIdentity();
        shaderMgr.setUniformTransform4f(locModel, modelTrans);
        shaderMgr.setUniformTransform4f(locView, viewTrans);
        shaderMgr.setUniformTransform4f(locProj, projTrans);

        /*
        GLfloat greenValue = (sin(getTimeMS() * 0.002) / 2) + 0.5;
        UniLoc colorLoc = shaderMgr.getUniformLocation(shaderProgramHDL, "ourColor");
        shaderMgr.setUniform4f(colorLoc, 0.0f, greenValue, 0.0f, 1.0f);
        */
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};


int main(int argc, char *argv[]) {
    // rayTraceMain(argc, argv);
    // return 0;
	MyContext context;
	context.setup(800, 600);
	context.loop();
	return 0;
}
