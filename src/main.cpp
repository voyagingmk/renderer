#include "stdafx.h"
#include "main_raytrace.hpp"
#include "realtime/shadermgr.hpp"
#include "realtime/texturemgr.hpp"
#include "realtime/context.hpp"
#include "transform.hpp"
#include "geometry.cpp"
#include "quaternion.hpp"
#include "buffermgr.hpp"
#include "realtime/glutils.hpp"
#include "realtime/model.hpp"
#include "camera.hpp"

using namespace std;
using namespace renderer;


const GLuint SHADOW_WIDTH = 800, SHADOW_HEIGHT = 600;
GLuint quadVAO = 0;
GLuint quadVBO;
GLuint planeVAO = 0;
GLuint planeVBO;


void RenderQuad()
{
    if (quadVAO == 0)
    {
        float len = 0.5f;
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -len,  len, 0.0f,  0.0f, 1.0f,
            -len, -len, 0.0f,  0.0f, 0.0f,
            len,  len, 0.0f,  1.0f, 1.0f,
            len, -len, 0.0f,  1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

class MyContext : public RendererContextSDL {
    PerspectiveCamera camera;
    FrameBuf depthFrameBuf;
    FrameBuf mainFrameBuf;
    ShaderProgramHDL mainHDL;
    ShaderProgramHDL depthMapHDL;
    ShaderProgramHDL singleColorHDL;
    ShaderProgramHDL screenHDL;
    TexID texID1, texID2;
    std::map<SDL_Keycode, uint8_t> keyState;
    std::vector<Model*> objs;
    Model* terrian;
    Model* quad;
    PhongMaterial* material;
    PointLight* light;
    FrameBuf depthMapBuf;

public:
    MyContext():
        mainHDL(0),
        texID1(0), texID2(0) {}
	virtual void onSDLEvent(SDL_Event& e) override {
        switch (e.type) {
            case SDL_QUIT:
                shouldExit = true;
                break;
            default:
                break;
        }
	}
    virtual void onSDLKeyboardEvent(SDL_KeyboardEvent& e) override {
        auto k = e.keysym.sym;
        keyState[k] = e.state;
        if(e.state == SDL_PRESSED) {
            switch (k) {
                case SDLK_ESCAPE: {
                    shouldExit = true;
                    break;
                }
                default:
                    break;
            }
        }
    }
    virtual void onCustomSetup() override {
        // Configure depth map FBO
        glGenFramebuffers(1, &depthMapBuf.fboID);
        // - Create depth texture
        glGenTextures(1, &depthMapBuf.depthTexID);
        glBindTexture(GL_TEXTURE_2D, depthMapBuf.depthTexID);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapBuf.fboID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapBuf.depthTexID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        BufferMgrOpenGL& bufferMgr = BufferMgrOpenGL::getInstance();
       
        depthFrameBuf = bufferMgr.CreateDepthFrameBuffer(winWidth, winHeight);
        // depthFrameBuf = bufferMgr.CreateColorFrameBuffer(winWidth, winHeight, BufType::Tex, 0);
        mainFrameBuf = bufferMgr.CreateColorFrameBuffer(winWidth, winHeight, BufType::Tex, 0);
        
        texMgr.setTextureDirPath("assets/images/");
        shaderMgr.setShaderFileDirPath("assets/shaders/");
        mainHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "main.vs" },
            { ShaderType::Fragment, "main.fs"}
        });
        depthMapHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "depthMap.vs" },
            { ShaderType::Fragment, "depthMap.fs"}
        });
        singleColorHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "main.vs" },
            { ShaderType::Fragment, "singlecolor.fs"}
        });
        screenHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "screen.vs" },
            { ShaderType::Fragment, "screen.fs"}
        });
        
        if (!depthMapHDL || !mainHDL || !singleColorHDL || !screenHDL) {
            shutdown("createShaderProgram failed");
        }
        
        texID1 = texMgr.loadTexture("dog.png", "tex1");
        texID2 = texMgr.loadTexture("terrian.png", "tex2");
        
        auto matPool = GetPool<PhongMaterial>();
        material = matPool->newElement(
            Color(1.0f, 0.5f, 0.31f),
            Color(1.0f, 0.5f, 0.31f),
            Color(1.0f, 1.0f, 1.0f),
            32.0f);
        
        auto lightPool = GetPool<PointLight>();
        light = lightPool->newElement(Vector3dF(0.0f, 20.0f, 10.0f));
        light->ambient = Color(1.0f, 1.0f, 1.0f);
        light->diffuse = Color(1.0f, 1.0f, 1.0f);
        light->specular = Color(1.0f, 1.0f, 1.0f);
        light->constant = 1.0f;
        light->linear = 0.014f;
        light->quadratic = 0.0007f;

        auto pool = GetPool<Model>();
        string dirPath = "./assets/models/";

        for(int i = 0; i < 1; i++) {
            Model* model = pool->newElement();
            objs.push_back(model);
            model->CustomInit(dirPath + "dog.obj");
            model->SetScale(Vector3dF(0.1f, 0.1f, 0.1f));
            //model->SetPos(Vector3dF(-10.0f + i * 3.0f, 10.0f, -30.0f - i * 100.0f));
            model->SetPos({0.0, 0.0, -1.0});
            model->SetRotate(90, Axis::y);
        }
        terrian = pool->newElement();
        terrian->CustomInit(dirPath + "plane.obj");
        terrian->SetScale({10.0, 10.0, 10.0});
        terrian->SetPos({0.0, 0.0, -10.0});
        quad = pool->newElement();
        Mesh mesh;
        Vertex v;
        v.position =  {-1.0f,  1.0f, 0.0f}; // Left Top
        v.texCoords = {0.0f, 1.0f};
        mesh.vertices.push_back(v);
        v.position =  {-1.0f, -1.0f, 0.0f}; // Left Bottom
        v.texCoords = {0.0f, 0.0f};
        mesh.vertices.push_back(v);
        v.position =  {1.0f,  -1.0f, 0.0f}; // Right Bottom
        v.texCoords = {1.0f, 0.0f};
        mesh.vertices.push_back(v);
        v.position =  {1.0f,  1.0f, 0.0f}; // Right Top
        v.texCoords = {1.0f, 1.0f};
        mesh.vertices.push_back(v);
        mesh.indexes = {
            0, 1, 2,
            0, 2, 3
        };
        quad->CustomInit(mesh);
        
        checkSDLError();
        CheckGLError;
        camera.SetFov(45.0f);
        camera.SetAspect((float)winWidth / (float)winHeight);
        camera.SetNear(0.01f);
        camera.SetFar(10000.0f);
        camera.SetCameraPosition(Vector3dF(0.0f, 10.0f, 0.0f));
       
        // Setup OpenGL options
        //glDepthFunc(GL_LESS);
        //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
       
        
    }
    void updateCamera() {
        Vector3dF p = light->pos;
        //Vector3dF p = camera.GetCameraPosition();
        float dis = 0.2f;
        if(keyState[SDLK_w] == SDL_PRESSED) {
            p += {0.0f, 0.0f, -dis};
        }
        if(keyState[SDLK_s] == SDL_PRESSED) {
            p += {0.0f, 0.0f, dis};
        }
        if(keyState[SDLK_a] == SDL_PRESSED) {
            p += {-dis, 0.0f, 0.0f};
        }
        if(keyState[SDLK_d] == SDL_PRESSED) {
            p += {dis, 0.0f, 0.0f};
        }
        if(keyState[SDLK_q] == SDL_PRESSED) {
            p += {0.0f, dis, 0.0f};
        }
        if(keyState[SDLK_e] == SDL_PRESSED) {
            p += {0.0f, -dis, 0.0f};
        }
        if(p != camera.GetCameraPosition()) {
            printf("eye: %.3f, %.3f, %.3f\n", p.x, p.y, p.z);
        }
        light->pos = p;
        //camera.SetCameraPosition(p);
        //camera.SetTargetVector(p + Vector3dF(0.0, 0.0, -1.0));
        
        if(keyState[SDLK_y] == SDL_PRESSED) {
            float f = camera.GetFar();
            f += 0.1f;
            camera.SetFar(f);
        }

        if(keyState[SDLK_u] == SDL_PRESSED) {
            float f = camera.GetFar();
            f -= 0.1f;
            camera.SetFar(f);
        }
    }
    
    virtual void onPoll() override
    {   glEnable(GL_DEPTH_TEST);
        //glDepthRangef(0.0f, 1.0f);
        updateWorld();
        updateCamera();
        BufferMgrOpenGL& buffMgr = BufferMgrOpenGL::getInstance();
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        
        // 1. first render to depth map
        buffMgr.UseFrameBuffer(depthFrameBuf);
        Matrix4x4 lightProj = Ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 50.0f);
        Matrix4x4 lightPV = lightProj * LookAt(light->pos, Vector3dF(0.0, 0.0, 0.0), {0.0f, 1.0f, 0.0f});
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  depthFrameBuf,
                  depthMapHDL,
                  light->pos,
                  lightPV);
        buffMgr.UnuseFrameBuffer(depthFrameBuf);
        
        
        buffMgr.UseFrameBuffer(mainFrameBuf);
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  mainFrameBuf,
                  mainHDL,
                  camera.GetCameraPosition(),
                  lightPV);
        buffMgr.UnuseFrameBuffer(mainFrameBuf);
        
        
        
        Shader& screenshader = shaderMgr.getShader(screenHDL);
        screenshader.use();
        glViewport(0, 0, winWidth, winHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        texMgr.activateTexture(0, depthFrameBuf.depthTexID);
       // texMgr.activateTexture(0, mainFrameBuf.getTexID());
       // texMgr.activateTexture(0, mainFrameBuf.depthTexID);
        quad->Draw();
        CheckGLError;
        
    }
    
    void updateWorld() {
        static float angle = 0.0f;
        angle += 0.2f;
        for(int i = 0; i < objs.size(); i++) {
            Model* obj = objs[i];
            auto oldScale = obj->scale;
            obj->SetRotate(angle, Axis::y);
        }
    }
    
    void drawTerrian(Shader& shader) {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        // 地面
        texMgr.activateTexture(0, texID2);
        shader.set1i("texture1", 0);
        
        shader.setMatrix4f("model", terrian->o2w->m);
        shader.setMatrix4f("normalMat", terrian->o2w->mInv.transpose());
        terrian->Draw();
    }
    
    void drawObjs(Shader& shader, float scale = 1.0f) {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        texMgr.activateTexture(0, texID1);
        shader.set1i("texture1", 0);
        
        for(int i = 0; i < objs.size(); i++) {
            Model* obj = objs[i];
            auto oldScale = obj->scale;
            obj->SetScale(oldScale * scale);
            shader.setMatrix4f("model", obj->o2w->m);
            shader.setMatrix4f("normalMat", obj->o2w->mInv.transpose());
            obj->Draw();
            obj->SetScale(oldScale);
        }
    }
    void useShader(Shader& shader, const Vector3dF viewPos, const Matrix4x4& PV) {
        shader.use();
        shader.set3f("viewPos", viewPos);
        shader.setMatrix4f("PV", PV);
    }
    void drawScene(Color clearColor,
                   uint32_t clearBits,
                   FrameBuf& buf,
                   ShaderProgramHDL hdl,
                   const Vector3dF viewPos,
                   const Matrix4x4& PV) {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        Shader& shader = shaderMgr.getShader(hdl);
        useShader(shader, viewPos, PV);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, buf.width, buf.height);
        glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
        glClear(clearBits);
        shader.setLight(light);
        shader.setMaterial(material);
        static float angle = 0.0f;
        angle += 1.0f;
        shader.setMatrix4f("model", Translate<Matrix4x4>({0.0f, 0.0f, -10.0f}));
        //drawTest();
        drawTerrian(shader);
        drawObjs(shader);
    }
    void drawTest(){
        if(!planeVAO){
            float len = 1000.0f;
            float s1 = -1.0f, s2 = -1.0f;
            GLfloat planeVertices[] = {
                // Positions            // Normals           // Texture Coords
                 len,  s1,  len, 0.0f,  1.0f,  0.0f,  25.0f, 0.0f,
                -len,  s1, -len, 0.0f,  1.0f,  0.0f,  0.0f,  25.0f,
                -len,  s1,  len, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
                
                 len,  s2,  len, 0.0f,  1.0f,  0.0f,  25.0f, 0.0f,
                 len,  s2, -len, 0.0f,  1.0f,  0.0f,  25.0f, 25.0f,
                -len,  s2, -len, 0.0f,  1.0f,  0.0f,  0.0f,  25.0f
            };
            // Setup plane VAO
            glGenVertexArrays(1, &planeVAO);
            glGenBuffers(1, &planeVBO);
            glBindVertexArray(planeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
            glBindVertexArray(0);
        }
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
   
    }
    /*
    void draw2() {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        Shader& mainShader = shaderMgr.getShader(mainHDL);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glViewport(0, 0, mainFrameBuf.width, mainFrameBuf.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        useShader(mainShader);
        
        glStencilMask(0x00);
        drawTerrian(mainShader);
        
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        useShader(mainShader);
        drawObjs(mainShader);
        
        Shader& singleColorShader = shaderMgr.getShader(singleColorHDL);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00); // disable writing to the stencil buffer
        glDisable(GL_DEPTH_TEST);
        useShader(singleColorShader);
        drawObjs(singleColorShader, 1.02f);
        
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
    }*/
};


int main(int argc, char *argv[]) {
    // rayTraceMain(argc, argv);
    // return 0;
	MyContext context;
	context.setup(800, 600);
	context.loop();
	return 0;
}
