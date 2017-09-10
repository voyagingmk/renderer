#include "stdafx.h"
#include "main_realtime.hpp"
#include "realtime/shadermgr.hpp"
#include "realtime/texturemgr.hpp"
#include "realtime/context.hpp"
#include "com/transform.hpp"
#include "com/geometry.hpp"
#include "com/quaternion.hpp"
#include "buffermgr.hpp"
#include "realtime/glutils.hpp"
#include "realtime/model.hpp"
#include "realtime/textmgr.hpp"
#include "camera.hpp"
#include "parser.hpp"
#include "ecs/test.hpp"

using namespace std;
using namespace renderer;
using json = nlohmann::json;

enum class RenderType
{
    Normal = 1,
    DepthMap = 2
};

class MyContext : public RendererContextSDL
{
    PerspectiveCamera camera;
    FrameBuf depthFrameBuf;
    FrameBuf mainFrameBuf;
    std::map<SDL_Keycode, uint8_t> keyState;
    std::vector<Model *> objs;
    Model *terrian;
    Model *quad;
    Model *lightObj;
    PointLight *light;
    Lights lights;
    FrameBuf depthMapBuf;
    float yaw;
    float pitch;
    Vector2dF cameraVec;
    RenderType renderType;

  public:
    MyContext() : yaw(-90.0f), pitch(0),
                  renderType(RenderType::Normal)
    {
    }

    virtual void onSDLEvent(SDL_Event &e) override
    {
        switch (e.type)
        {
        case SDL_QUIT:
            shouldExit = true;
            break;
        default:
            break;
        }
    }

    virtual void onSDLMouseEvent(SDL_MouseMotionEvent &e) override
    {
        //printf("mMotion, t:%u, state:%u, (%d,%d), rel:(%d,%d)\n", e.type, e.state, e.x, e.y, e.xrel, e.yrel);
        float scale = 0.3f;
        cameraVec = {scale * e.xrel, -scale * e.yrel};
        yaw += cameraVec.x;
        pitch += cameraVec.y;
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        Vector3dF front;
        front.x = cos(Radians(yaw)) * cos(Radians(pitch)); // 0
        front.y = sin(Radians(pitch));                     // 0
        front.z = sin(Radians(yaw)) * cos(Radians(pitch)); // -1
        front = front.Normalize();
        camera.SetTargetVector(camera.GetCameraPosition() + front);
    }

    virtual void onSDLMouseEvent(SDL_MouseButtonEvent &e) override
    {
        printf("mButton, t:%u, state:%u, btn:%u, (%d,%d), clicks:%d\n", e.type, e.state, e.button, e.x, e.y, e.clicks);
    }

    virtual void onSDLKeyboardEvent(SDL_KeyboardEvent &e) override
    {
        auto k = e.keysym.sym;
        keyState[k] = e.state;
        if (e.state == SDL_PRESSED)
        {
            switch (k)
            {
            case SDLK_ESCAPE:
            {
                shouldExit = true;
                break;
            }
            case SDLK_1:
            {
                // normal
                renderType = RenderType::Normal;
                break;
            }
            case SDLK_2:
            {
                // depth map
                renderType = RenderType::DepthMap;
                break;
            }
            default:
                break;
            }
        }
    }

    virtual void onCustomSetup() override
    {

		static ECSDemo demo;
		demo.setup();
		demo.update(0.1f);

        TextureMgr &texMgr = TextureMgr::getInstance();
        ShaderMgr &shaderMgr = ShaderMgr::getInstance();
        BufferMgr &bufferMgr = BufferMgr::getInstance();
        MaterialMgr &matMgr = MaterialMgr::getInstance();

        SceneParser parser;
        json config = readJson("demo.json");
        //TexRef texRef = texMgr.CreateDepthTexture(DepthTexType::DepthStencil, 1024, 1024);
        //depthFrameBuf = bufferMgr.CreateDepthFrameBuffer(DepthTexType::DepthStencil, texRef);
        TexRef texRef = texMgr.CreateDepthTexture(DepthTexType::CubeMap, 1024, 1024);
        CheckGLError;
        depthFrameBuf = bufferMgr.CreateDepthFrameBuffer(DepthTexType::CubeMap, texRef);
        CheckGLError;
        mainFrameBuf = bufferMgr.CreateColorFrameBuffer(winWidth, winHeight, BufType::Tex, 0);
        std::string assetsDir = config["assetsDir"];
        std::string texSubDir = config["texSubDir"];
        std::string shaderSubDir = config["shaderSubDir"];
        texMgr.setTextureDirPath((assetsDir + texSubDir).c_str());
        shaderMgr.setShaderFileDirPath((assetsDir + shaderSubDir).c_str());
        parser.parseShaders(config);
        parser.parseTextures(config);
        parser.parseLights(config, lights);

        auto lightPool = GetPool<PointLight>();
        light = lightPool->newElement(Vector3dF(0.0f, 20.0f, 0.0f));
        light->ambient = Color(0.5f, 0.5f, 0.5f);
        light->diffuse = Color(1.0f, 1.0f, 1.0f);
        light->specular = Color(1.0f, 1.0f, 1.0f);
        light->constant = 1.0f;
        light->linear = 0.014f;
        light->quadratic = 0.0007f;

        auto pool = GetPool<Model>();
        string dirPath = "./assets/models/";

        auto matPool = GetPool<MaterialPhong>();
        Material *mat = matPool->newElement();
        MaterialSettingID sID = matMgr.addMaterialSetting(GetPool<MaterialSettingPhong>()->newElement(
            Color(0.3f, 0.3f, 0.3f),
            Color(1.0f, 1.0f, 1.0f),
            Color(1.0f, 1.0f, 1.0f),
            32.0f, 0));
        mat->bindSetting(sID);
        MaterialID matID = matMgr.addMaterial(mat);

        lightObj = pool->newElement();
        lightObj->CustomInit(dirPath + "cube.obj");
        lightObj->SetScale(Vector3dF(1.0f, 1.0f, 1.0f));
        lightObj->SetPos(light->pos);
        lightObj->matID = matID;

        for (int i = 0; i < 3; i++)
        {
            Model *model = pool->newElement();
            objs.push_back(model);
            model->CustomInit(dirPath + "dog.obj");
            //model->SetScale(Vector3dF(2.0f, 2.0f, 2.0f));
            model->SetScale(Vector3dF(0.1f, 0.1f, 0.1f));
            model->SetPos(Vector3dF(-10.0f + i * 10.0f, 0.0f, -1.0f));
            model->SetRotate(90, Axis::y);
            model->matID = matID;
        }

        mat = matPool->newElement();
        sID = matMgr.addMaterialSetting(GetPool<MaterialSettingPhong>()->newElement(
            Color(0.9f, 0.9f, 0.9f),
            Color(1.0f, 1.0f, 1.0f),
            Color(1.0f, 1.0f, 1.0f),
            64.0f, 0));
        mat->bindSetting(sID);
        matID = matMgr.addMaterial(mat);

        terrian = pool->newElement();
        terrian->CustomInit(dirPath + "plane.obj");
        //terrian->SetScale({10.0, 10.0, 10.0});
        terrian->SetPos({0.0, 0.0, 0.0});
        terrian->matID = matID;

        quad = pool->newElement();
        Mesh mesh;
        Vertex v;
        v.position = {-1.0f, 1.0f, 0.0f}; // Left Top
        v.texCoords = {0.0f, 1.0f};
        mesh.vertices.push_back(v);
        v.position = {-1.0f, -1.0f, 0.0f}; // Left Bottom
        v.texCoords = {0.0f, 0.0f};
        mesh.vertices.push_back(v);
        v.position = {1.0f, -1.0f, 0.0f}; // Right Bottom
        v.texCoords = {1.0f, 0.0f};
        mesh.vertices.push_back(v);
        v.position = {1.0f, 1.0f, 0.0f}; // Right Top
        v.texCoords = {1.0f, 1.0f};
        mesh.vertices.push_back(v);
        mesh.indexes = {
            0, 1, 2,
            0, 2, 3};
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

     //   TextMgr &textmgr = TextMgr::getInstance();
      //  textmgr.setScreenSize(winWidth, winHeight);
      //  textmgr.loadTTF("./assets/fonts/STHeiti Light.ttc", 60);
      //  textmgr.setupBuffer();
       // textmgr.test('A', 60, 60, 200, 200);
    }

    void updateCamera()
    {
        Vector3dF p = {0.0f, 0.0f, 0.0f};
        if (keyState[SDLK_w] == SDL_PRESSED)
        {
            p = camera.GetFrontVector().Normalize();
        }
        if (keyState[SDLK_s] == SDL_PRESSED)
        {
            p = -camera.GetFrontVector().Normalize();
        }
        if (keyState[SDLK_a] == SDL_PRESSED)
        {
            p = -camera.GetRightVector().Normalize();
        }
        if (keyState[SDLK_d] == SDL_PRESSED)
        {
            p = camera.GetRightVector().Normalize();
        }
        if (keyState[SDLK_q] == SDL_PRESSED)
        {
            p = camera.GetUpVector().Normalize();
        }
        if (keyState[SDLK_e] == SDL_PRESSED)
        {
            p = -camera.GetUpVector().Normalize();
        }
        if (p != camera.GetCameraPosition())
        {
            //printf("eye: %.3f, %.3f, %.3f\n", p.x, p.y, p.z);
        }
        camera.SetTargetVector(camera.GetTargetVector() + p * 0.2f);
        camera.SetCameraPosition(camera.GetCameraPosition() + p * 0.2f);
        //camera.SetTargetVector(p + Vector3dF(0.0, 0.0, -1.0));

        if (keyState[SDLK_y] == SDL_PRESSED)
        {
            float f = camera.GetFar();
            f += 0.1f;
            camera.SetFar(f);
        }

        if (keyState[SDLK_u] == SDL_PRESSED)
        {
            float f = camera.GetFar();
            f -= 0.1f;
            camera.SetFar(f);
        }
    }

    virtual void onPoll() override
    {
        updateWorld();
        updateCamera();
        BufferMgr &buffMgr = BufferMgr::getInstance();
        TextureMgr &texMgr = TextureMgr::getInstance();
        ShaderMgr &shaderMgr = ShaderMgr::getInstance();

        GLfloat aspect = depthFrameBuf.width / depthFrameBuf.height;
        GLfloat near = 0.1f;
        GLfloat far = 10000.0f;
        Matrix4x4 shadowProj = Perspective(90.0f, aspect, near, far);
        std::vector<Matrix4x4> lightPVs;
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{1.0, 0.0, 0.0}, {0.0, -1.0, 0.0}));  // right
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{-1.0, 0.0, 0.0}, {0.0, -1.0, 0.0})); // left
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}));   // top
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{0.0, -1.0, 0.0}, {0.0, 0.0, -1.0})); // bottom
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{0.0, 0.0, 1.0}, {0.0, -1.0, 0.0}));  // near
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{0.0, 0.0, -1.0}, {0.0, -1.0, 0.0})); // far

        // 1. first render to depth map
        buffMgr.UseFrameBuffer(depthFrameBuf);
        Matrix4x4 lightProj = Ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 100.0f);
        Matrix4x4 lightPV = lightProj * LookAt(light->pos, Vector3dF(0.0, 0.0, 0.0), {0.0f, 1.0f, 0.0f});

        CheckGLError;
        Shader &depthMapShader = shaderMgr.getShader("depthMap");
        depthMapShader.use();
        CheckGLError;
        depthMapShader.setMatrixes4f("lightPVs", lightPVs);
        depthMapShader.set1f("far_plane", far);
        depthMapShader.set3f("lightPos", light->pos);

        CheckGLError;
        glCullFace(GL_FRONT);
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_DEPTH_BUFFER_BIT,
                  depthFrameBuf,
                  "depthMap");
        glCullFace(GL_BACK);
        buffMgr.UnuseFrameBuffer(depthFrameBuf);

        buffMgr.UseFrameBuffer(mainFrameBuf);
        Shader &mainShader = shaderMgr.getShader("main");
        mainShader.use();
        mainShader.set1f("far_plane", far);
        mainShader.setMatrix4f("lightPV", lightPV);
        mainShader.set1i("depthMap", 1);
        texMgr.activateTexture(1, depthFrameBuf.depthTex);

        CheckGLError;
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  mainFrameBuf,
                  "main",
                  camera.GetCameraPosition(),
                  camera.GetMatrix());
        drawLight(mainShader);
        buffMgr.UnuseFrameBuffer(mainFrameBuf);

        glViewport(0, 0, winWidth, winHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        if (renderType == RenderType::Normal)
        {
            Shader &screenShader = shaderMgr.getShader("screen");
            screenShader.use();
            screenShader.set1i("texture1", 0);
            texMgr.activateTexture(0, mainFrameBuf.getTexRef());
        }
        else
        {
            Shader &screenShader = shaderMgr.getShader("depthMapDebug");
            screenShader.use();
            screenShader.set1i("texture1", 0);
            texMgr.activateTexture(0, depthFrameBuf.depthTex);
        }
        //texMgr.activateTexture(0, mainFrameBuf.depthTexID);
        quad->SetScale({1.0f, 1.0f, 1.0f});
        quad->SetPos({0.0f, 0.0f, 0.0f});
        quad->Draw();

        TextMgr &textmgr = TextMgr::getInstance();

        quad->SetScale({200.0f, 200.0f, 1.0f});
        quad->SetPos({winWidth * 0.5f, winHeight * 0.5f, 0.0f});
        Shader &sdfShader = shaderMgr.getShader("sdf");
        sdfShader.use();
        sdfShader.set1i("texture1", 0);
        Matrix4x4 PV = Ortho(0.0f, (float)winWidth, 0.0f, (float)winHeight, 0.01f, 100.0f);
        sdfShader.setMatrix4f("PV", PV);
        sdfShader.setMatrix4f("model", quad->o2w->m);
        texMgr.activateTexture(0, "sdf");

        quad->Draw();

        textmgr.RenderText<std::u16string>(shaderMgr.getShader("text"), u"测试", 0.0f, 10.0f, 1.0f, Color(0.5, 0.8f, 0.2f));
        CheckGLError;
        textmgr.RenderText<std::string>(shaderMgr.getShader("text"), "(C) LearnOpenGL.com", 540.0f, 570.0f, 1.0f, Color(0.3, 0.7f, 0.9f));
        CheckGLError;
    }

    void updateWorld()
    {
        /*
        static float angle = 0.0f;
        angle += 0.2f;
        for(int i = 0; i < objs.size(); i++) {
            Model* obj = objs[i];
            auto oldScale = obj->scale;
            obj->SetRotate(angle, Axis::y);
        }*/

        static float angle = 0.0f;
        angle += 0.3f;
        Vector3dF p = 15.0f * Vector3dF(cos(Radians(angle)), 1.0f, sin(Radians(angle)));
        p.y = 1.0f;
        light->pos = p;
        lightObj->SetPos(p);
    }

    void drawLight(Shader &shader)
    {
        lightObj->Draw(&shader);
    }

    void drawTerrian(Shader &shader)
    {
        TextureMgr &texMgr = TextureMgr::getInstance();
        // 地面
        texMgr.activateTexture(0, "terrian");
        texMgr.activateTexture(2, "terrianNorm");
        shader.set1i("texture1", 0);
        shader.set1i("normTex", 2);
        terrian->Draw(&shader);
    }

    void drawObjs(Shader &shader, float scale = 1.0f)
    {
        TextureMgr &texMgr = TextureMgr::getInstance();

        texMgr.activateTexture(0, "dog");
        //texMgr.DisableTexture(2); // no normal map
        shader.set1i("texture1", 0);
        for (int i = 0; i < objs.size(); i++)
        {
            Model *obj = objs[i];
            auto oldScale = obj->scale;
            obj->SetScale(oldScale * scale);
            obj->Draw(&shader);
            obj->SetScale(oldScale);
        }
    }
    void drawScene(Color clearColor,
                   uint32_t clearBits,
                   FrameBuf &buf,
                   const char *shaderALias,
                   const Vector3dF viewPos = {0.0, 0.0, 0.0},
                   const Matrix4x4 PV = Matrix4x4())
    {
        ShaderMgr &shaderMgr = ShaderMgr::getInstance();
        Shader &shader = shaderMgr.getShader(shaderALias);
        shader.set3f("viewPos", viewPos);
        shader.setMatrix4f("PV", PV);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, buf.width, buf.height);
        glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
        glClear(clearBits);
        shader.setLight(light);
        drawTerrian(shader);
        drawObjs(shader);
        CheckGLError;
    }
};

int realtimeMain(int argc, char *argv[])
{
    MyContext context;
    context.setup(800, 600);
    context.loop();
    return 0;
}
