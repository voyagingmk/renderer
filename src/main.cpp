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

enum class RenderType {
    Normal = 1,
    DepthMap = 2
};

class MyContext : public RendererContextSDL {
    PerspectiveCamera camera;
    FrameBuf depthFrameBuf;
    FrameBuf mainFrameBuf;
    ShaderProgramHDL mainHDL;
    ShaderProgramHDL depthMapHDL;
    ShaderProgramHDL depthMapDebugHDL;
    ShaderProgramHDL singleColorHDL;
    ShaderProgramHDL screenHDL;
    ShaderProgramHDL pointDepthMapHDL;
    TexRef tex1, tex2;
    std::map<SDL_Keycode, uint8_t> keyState;
    std::vector<Model*> objs;
    Model* terrian;
    Model* quad;
    PhongMaterial* material;
    PointLight* light;
    FrameBuf depthMapBuf;
    float yaw;
    float pitch;
    Vector2dF cameraVec;
    RenderType renderType;
public:
    MyContext():
        mainHDL(0),
        yaw(-90.0f), pitch(0),
        renderType(RenderType::Normal)
         {}
	virtual void onSDLEvent(SDL_Event& e) override {
        switch (e.type) {
            case SDL_QUIT:
                shouldExit = true;
                break;
            default:
                break;
        }
	}
    
    virtual void onSDLMouseEvent(SDL_MouseMotionEvent& e) override {
       //printf("mMotion, t:%u, state:%u, (%d,%d), rel:(%d,%d)\n", e.type, e.state, e.x, e.y, e.xrel, e.yrel);
        float scale = 0.3f;
        cameraVec = {scale * e.xrel, -scale * e.yrel};
        yaw   += cameraVec.x;
        pitch += cameraVec.y;
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        Vector3dF front;
        front.x = cos(Radians(yaw)) * cos(Radians(pitch)); // 0
        front.y = sin(Radians(pitch)); // 0
        front.z = sin(Radians(yaw)) * cos(Radians(pitch)); // -1
        front = front.Normalize();
        camera.SetTargetVector(camera.GetCameraPosition() + front);
    }
    virtual void onSDLMouseEvent(SDL_MouseButtonEvent& e) override {
          printf("mButton, t:%u, state:%u, btn:%u, (%d,%d), clicks:%d\n", e.type, e.state, e.button, e.x, e.y, e.clicks);
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
                case SDLK_1: {
                    // normal
                    renderType = RenderType::Normal;
                    break;
                }
                case SDLK_2: {
                    // depth map
                    renderType = RenderType::DepthMap;
                    break;
                }
                default:
                    break;
            }
        }
    }
    virtual void onCustomSetup() override {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        BufferMgrOpenGL& bufferMgr = BufferMgrOpenGL::getInstance();
        TexRef texRef = texMgr.CreateDepthTexture(DepthTexType::DepthStencil, 1024, 1024);
        depthFrameBuf = bufferMgr.CreateDepthFrameBuffer(DepthTexType::DepthStencil, texRef);
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
        depthMapDebugHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "depthMapDebug.vs" },
            { ShaderType::Fragment, "depthMapDebug.fs"}
        });
        pointDepthMapHDL = shaderMgr.createShaderProgram({
            { ShaderType::Geometry, "point_shadows_depth.gs"},
            { ShaderType::Vertex, "point_shadows_depth.vs" },
            { ShaderType::Fragment, "point_shadows_depth.fs"}
        });
        
        if (!depthMapHDL ||
            !mainHDL ||
            !singleColorHDL ||
            !screenHDL ||
            !depthMapDebugHDL||
            !pointDepthMapHDL) {
            shutdown("createShaderProgram failed");
        }
        
        tex1 = texMgr.loadTexture("dog.png", "tex1");
        tex2 = texMgr.loadTexture("terrian.png", "tex2");
        
        auto matPool = GetPool<PhongMaterial>();
        material = matPool->newElement(
            Color(0.3f, 0.3f, 0.3f),
            Color(1.0f, 1.0f, 1.0f),
            Color(1.0f, 1.0f, 1.0f),
            32.0f);
        
        auto lightPool = GetPool<PointLight>();
        light = lightPool->newElement(Vector3dF(5.0f, 5.0f, 5.0f));
        light->ambient = Color(0.5f, 0.5f, 0.5f);
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
        //terrian->SetScale({10.0, 10.0, 10.0});
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
        camera.SetCameraPosition(Vector3dF(0.0f, 15.0f, 25.0f));
       
        // Setup OpenGL options
        //glDepthFunc(GL_LESS);
        //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
       
        
    }
    void updateCamera() {
       // Vector3dF p = light->pos;
        Vector3dF p = {0.0f, 0.0f, 0.0f};
        if(keyState[SDLK_w] == SDL_PRESSED) {
            p = camera.GetFrontVector().Normalize();
        }
        if(keyState[SDLK_s] == SDL_PRESSED) {
            p =-camera.GetFrontVector().Normalize();
        }
        if(keyState[SDLK_a] == SDL_PRESSED) {
            p =-camera.GetRightVector().Normalize();
        }
        if(keyState[SDLK_d] == SDL_PRESSED) {
            p = camera.GetRightVector().Normalize();
        }
        if(keyState[SDLK_q] == SDL_PRESSED) {
            p = camera.GetUpVector().Normalize();
        }
        if(keyState[SDLK_e] == SDL_PRESSED) {
            p =-camera.GetUpVector().Normalize();
        }
        if(p != camera.GetCameraPosition()) {
            //printf("eye: %.3f, %.3f, %.3f\n", p.x, p.y, p.z);
        }
        //light->pos = p;
        camera.SetTargetVector(camera.GetTargetVector() + p * 0.2f);
        camera.SetCameraPosition(camera.GetCameraPosition() + p * 0.2f);
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
    {
        updateWorld();
        updateCamera();
        BufferMgrOpenGL& buffMgr = BufferMgrOpenGL::getInstance();
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        
        // 1. first render to depth map
        buffMgr.UseFrameBuffer(depthFrameBuf);
        Matrix4x4 lightProj = Ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 100.0f);
        Matrix4x4 lightPV = lightProj * LookAt(light->pos, Vector3dF(0.0, 0.0, 0.0), {0.0f, 1.0f, 0.0f});
        Shader& depthMapShader = shaderMgr.getShader(depthMapHDL);
        depthMapShader.use();
        depthMapShader.setMatrix4f("lightPV", lightPV);
        glCullFace(GL_FRONT);
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  depthFrameBuf,
                  depthMapHDL);
        glCullFace(GL_BACK);
        buffMgr.UnuseFrameBuffer(depthFrameBuf);
        
        
        buffMgr.UseFrameBuffer(mainFrameBuf);
        Shader& mainShader = shaderMgr.getShader(mainHDL);
        mainShader.use();
        mainShader.setMatrix4f("lightPV", lightPV);
        mainShader.set1i("texture2", 1);
        texMgr.activateTexture(1, depthFrameBuf.depthTexID);
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  mainFrameBuf,
                  mainHDL,
                  camera.GetCameraPosition(),
                  camera.GetMatrix());
        buffMgr.UnuseFrameBuffer(mainFrameBuf);
        

        glViewport(0, 0, winWidth, winHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        if (renderType == RenderType::Normal) {
            Shader& screenShader = shaderMgr.getShader(screenHDL);
            screenShader.use();
            screenShader.set1i("texture1", 0);
            texMgr.activateTexture(0, mainFrameBuf.getTexID());
        } else {
            Shader& screenShader = shaderMgr.getShader(depthMapDebugHDL);
            screenShader.use();
            screenShader.set1i("texture1", 0);
            texMgr.activateTexture(0, depthFrameBuf.depthTexID);
        }
        //texMgr.activateTexture(0, mainFrameBuf.depthTexID);
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
        texMgr.activateTexture(0, tex2);
        shader.set1i("texture1", 0);
        
        shader.setMatrix4f("model", terrian->o2w->m);
        shader.setMatrix4f("normalMat", terrian->o2w->mInv.transpose());
        terrian->Draw();
    }
    
    void drawObjs(Shader& shader, float scale = 1.0f) {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        texMgr.activateTexture(0, tex1);
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
    void drawScene(Color clearColor,
                   uint32_t clearBits,
                   FrameBuf& buf,
                   ShaderProgramHDL hdl,
                   const Vector3dF viewPos = {0.0, 0.0, 0.0},
                   const Matrix4x4 PV = Matrix4x4()) {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        Shader& shader = shaderMgr.getShader(hdl);
        shader.set3f("viewPos", viewPos);
        shader.setMatrix4f("PV", PV);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, buf.width, buf.height);
        glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
        glClear(clearBits);
        shader.setLight(light);
        shader.setMaterial(material);
        drawTerrian(shader);
        drawObjs(shader);
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
