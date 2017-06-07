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

class MyContext : public RendererContextSDL {
    PerspectiveCamera camera;
    ShaderProgramHDL shaderProgramHDL;
    ShaderProgramHDL singleColorHDL;
    texID texID1, texID2;
    std::map<SDL_Keycode, uint8_t> keyState;
    std::vector<Model*> objs;
    Model* terrian;
    PhongMaterial* material;
    PointLight* light;
public:
    MyContext():
        shaderProgramHDL(0),
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
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        texMgr.setTextureDirPath("assets/images/");
        shaderMgr.setShaderFileDirPath("assets/shaders/");
        shaderProgramHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "test1.vs" },
            { ShaderType::Fragment, "test1.fs"}
        });
        singleColorHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "test1.vs" },
            { ShaderType::Fragment, "singlecolor.fs"}
        });
        texID1 = texMgr.loadTexture("dog.png", "tex1");
        texID2 = texMgr.loadTexture("terrian.png", "tex2");
        
        auto matPool = GetPool<PhongMaterial>();
        material = matPool->newElement(
            Color(1.0f, 0.5f, 0.31f),
            Color(1.0f, 0.5f, 0.31f),
            Color(0.5f, 0.5f, 0.5f),
            32.0f);
        auto lightPool = GetPool<PointLight>();
        light = lightPool->newElement(Vector3dF(10.0f, 15.0f, -10.0f));
        light->ambient = Color(1.0f, 1.0f, 1.0f);
        light->diffuse = Color(1.0f, 1.0f, 1.0f);
        light->specular = Color(1.0f, 1.0f, 1.0f);
        light->constant = 1.0f;
        light->linear = 0.014f;
        light->quadratic = 0.0007f;
        
        glViewport(0, 0, winWidth, winHeight);

        auto pool = GetPool<Model>();
        string dirPath = "./assets/models/";

        for(int i = 0; i < 1; i++) {
            Model* model = pool->newElement();
            objs.push_back(model);
            model->CustomInit(dirPath + "dog.obj");
            model->SetScale(Vector3dF(0.1f, 0.1f, 0.1f));
            model->SetPos(Vector3dF(0.0f, 3.0f, -30.0f));
            model->SetRotate(90, Axis::y);
        }
        terrian = pool->newElement();
        terrian->CustomInit(dirPath + "plane.obj");
        
        checkSDLError();
        checkGLError();
        camera.SetFov(45.0f);
        camera.SetAspect((float)winWidth / (float)winHeight);
        camera.SetNear(0.01f);
        camera.SetFar(10000.0f);
        camera.SetCameraPosition(Vector3dF(0.0f, 10.0f, 0.0f));
       
        // Setup OpenGL options
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glDepthFunc(GL_LESS);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
    }
    void updateCamera() {
        Vector3dF p = camera.GetCameraPosition();
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
        camera.SetCameraPosition(p);
        camera.SetTargetVector(p + Vector3dF(0.0, 0.0, -1.0));
        
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
        draw();
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
        shader.set1i("ourTexture1", 0);
        
        shader.setMatrix4f("model", terrian->o2w->m);
        shader.setMatrix4f("normalMat", terrian->o2w->mInv.transpose());
        terrian->Draw();
    }
    
    void drawObjs(Shader& shader, float scale = 1.0f) {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        texMgr.activateTexture(0, texID1);
        shader.set1i("ourTexture1", 0);
        
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
    void useShader(Shader& shader) {
        shader.use();
        shader.set3f("viewPos", camera.GetCameraPosition());
        shader.setMatrix4f("PV", camera.GetMatrix());
        shader.setLight(light);
        shader.setMaterial(material);
    }
    void draw() {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Shader& mainShader = shaderMgr.getShader(shaderProgramHDL);
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
        drawObjs(singleColorShader, 1.1f);
        
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
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
