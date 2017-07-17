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
#include "realtime/textmgr.hpp"
#include "camera.hpp"
#include "sdf.hpp"


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
    ShaderProgramHDL textHDL;
    TexRef tex1, terrianTex, terrianNormTex;
    std::map<SDL_Keycode, uint8_t> keyState;
    std::vector<Model*> objs;
    Model* terrian;
    Model* quad;
    Model* lightObj;
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
        //TexRef texRef = texMgr.CreateDepthTexture(DepthTexType::DepthStencil, 1024, 1024);
        //depthFrameBuf = bufferMgr.CreateDepthFrameBuffer(DepthTexType::DepthStencil, texRef);
        TexRef texRef = texMgr.CreateDepthTexture(DepthTexType::CubeMap, 1024, 1024);
        CheckGLError;
        depthFrameBuf = bufferMgr.CreateDepthFrameBuffer(DepthTexType::CubeMap, texRef);
        CheckGLError;
        
        mainFrameBuf = bufferMgr.CreateColorFrameBuffer(winWidth, winHeight, BufType::Tex, 0);
        
        texMgr.setTextureDirPath("assets/images/");
        shaderMgr.setShaderFileDirPath("assets/shaders/");
        mainHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "point_shadow.vs" },
            { ShaderType::Fragment, "point_shadow.fs"}
        });
        depthMapHDL = shaderMgr.createShaderProgram({
            { ShaderType::Geometry, "point_shadows_depth.gs"},
            { ShaderType::Vertex, "point_shadows_depth.vs" },
            { ShaderType::Fragment, "point_shadows_depth.fs"}
        });
        /*mainHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "main.vs" },
            { ShaderType::Fragment, "main.fs"}
        });
        depthMapHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "depthMap.vs" },
            { ShaderType::Fragment, "depthMap.fs"}
        });*/
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
        
        textHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "text.vs" },
            { ShaderType::Fragment, "text.fs"}
        });
        
        if (!depthMapHDL ||
            !mainHDL ||
            !singleColorHDL ||
            !screenHDL ||
            !depthMapDebugHDL) {
            shutdown("createShaderProgram failed");
        }
        
        tex1 = texMgr.loadTexture("dog.png", "tex1", false);
        terrianTex = texMgr.loadTexture("brickwall.jpg", "terrianTex");
        terrianNormTex = texMgr.loadTexture("brickwall_normal.jpg", "terrianNormTex");
        
        
        auto matPool = GetPool<PhongMaterial>();
        
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
        
        Material* objMaterial = matPool->newElement(
            Color(0.3f, 0.3f, 0.3f),
            Color(1.0f, 1.0f, 1.0f),
            Color(1.0f, 1.0f, 1.0f),
            32.0f);
        
        lightObj = pool->newElement();
        lightObj->CustomInit(dirPath + "cube.obj");
        lightObj->SetScale(Vector3dF(1.0f, 1.0f, 1.0f));
        lightObj->SetPos(light->pos);
        lightObj->material = objMaterial;

        for(int i = 0; i < 3; i++) {
            Model* model = pool->newElement();
            objs.push_back(model);
            model->CustomInit(dirPath + "dog.obj");
            //model->SetScale(Vector3dF(2.0f, 2.0f, 2.0f));
            model->SetScale(Vector3dF(0.1f, 0.1f, 0.1f));
            model->SetPos(Vector3dF(-10.0f + i * 10.0f, 0.0f, -1.0f));
            model->SetRotate(90, Axis::y);
            model->material = objMaterial;
        }
        Material* terrianMaterial = matPool->newElement(
            Color(0.9f, 0.9f, 0.9f),
            Color(1.0f, 1.0f, 1.0f),
            Color(1.0f, 1.0f, 1.0f),
            64.0f);
        terrian = pool->newElement();
        terrian->CustomInit(dirPath + "plane.obj");
        //terrian->SetScale({10.0, 10.0, 10.0});
        terrian->SetPos({0.0, 0.0, 0.0});
        terrian->material = terrianMaterial;
        
        quad = pool->newElement();
        Mesh mesh;
        Vertex v;
        v.position = {-1.0f,  1.0f, 0.0f}; // Left Top
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
        
        TextMgr& textmgr = TextMgr::getInstance();
        textmgr.setScreenSize(winWidth, winHeight);
        textmgr.loadTTF("./assets/fonts/STHeiti Light.ttc", 16);
        textmgr.buildCharaTexture();
        textmgr.setupBuffer();
        FT_Error error;
        FT_GlyphSlot  slot = textmgr.face->glyph;  /* a small shortcut */
        int           pen_x, pen_y, n;
        const char* text = "S";
        const int num_chars = strlen(text);
        const int width = 18, height = 18;
        pen_x = 3;
        pen_y = 16;
        unsigned char Image[height][width] = {{0}};
        auto my_draw_bitmap = [&]( FT_Bitmap* bitmap, FT_Int x, FT_Int y)
        {
            FT_Int  i, j, p, q;
            FT_Int  x_max = x + bitmap->width;
            FT_Int  y_max = y + bitmap->rows;
            // bitmap->width bitmap->rows是单个字符的实际大小
            // (x, y)是字符在大图里的起始坐标
            printf("my_draw, w:%d, h:%d\n", bitmap->width, bitmap->rows);
            printf("my_draw, x:%d, y:%d\n", x, y);
            printf("x_max:%d, y_max:%d\n", x_max, y_max);
            // 左上角是原点，从上往下画,x,y都是++
            for ( j = y, q = 0; j < y_max; j++, q++ )
            {
                for ( i = x, p = 0; i < x_max; i++, p++ )
                {
                    if ( i<0 || j<0 || i>=width || j>=height )  continue;
                    Image[j][i] |= bitmap->buffer[q * bitmap->width + p];
                    if(Image[j][i]==0) // empty
                        printf("-");
                    else
                        printf("0");
                }
                printf("\n");
            }
        };
        for ( n = 0; n < num_chars; n++ )
        {
            /* load glyph image into the slot (erase previous one) */
            error = FT_Load_Char(textmgr.face, text[n], FT_LOAD_RENDER );
            if ( error )
                continue;  /* ignore errors */
            
            /* now, draw to our target surface */
            printf("bitmap: left: %d, top: %d\n", slot->bitmap_left, slot->bitmap_top);
            my_draw_bitmap( &slot->bitmap,
                           pen_x + slot->bitmap_left,
                           pen_y - slot->bitmap_top );
            
            /* increment pen position */
            pen_x += slot->advance.x >> 6;
        }

        int buffer[height][width];
        sdf::SDFBuilder builder(width, height);
        builder.buildSDF([&](int x, int y)->float {
            // 黑色返回0，白色返回1
            return Image[y][x] == 0? 1.0f : 0.0f;
            if(abs(x - y) <= 1) return 0.0f;
            return 1.0f;
        }, [&](int x, int y, float v){
            buffer[y][x] = v;
        });
        printf("result:\n");
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                printf("%d\t", buffer[y][x]);
            }
            printf("\n");
        }
    }
    
    void updateCamera() {
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

        
        GLfloat aspect = depthFrameBuf.width / depthFrameBuf.height;
        GLfloat near = 0.1f;
        GLfloat far = 10000.0f;
        Matrix4x4 shadowProj = Perspective(90.0f, aspect, near, far);
        std::vector<Matrix4x4> lightPVs;
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{ 1.0,  0.0,  0.0}, {0.0, -1.0,  0.0}));// right
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{-1.0,  0.0,  0.0}, {0.0, -1.0,  0.0}));// left
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{ 0.0,  1.0,  0.0}, {0.0,  0.0,  1.0}));// top
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{ 0.0, -1.0,  0.0}, {0.0,  0.0, -1.0}));// bottom
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{ 0.0,  0.0,  1.0}, {0.0, -1.0,  0.0}));// near
        lightPVs.push_back(shadowProj * LookAt(light->pos, light->pos + Vector3dF{ 0.0,  0.0, -1.0}, {0.0, -1.0,  0.0}));// far
        
        
        // 1. first render to depth map
        buffMgr.UseFrameBuffer(depthFrameBuf);
        Matrix4x4 lightProj = Ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 100.0f);
        Matrix4x4 lightPV = lightProj * LookAt(light->pos, Vector3dF(0.0, 0.0, 0.0), {0.0f, 1.0f, 0.0f});
        
        Shader& depthMapShader = shaderMgr.getShader(depthMapHDL);
        depthMapShader.use();
        depthMapShader.setMatrixes4f("lightPVs", lightPVs);
        depthMapShader.set1f("far_plane", far);
        depthMapShader.set3f("lightPos", light->pos);
 
        depthMapShader.setMatrix4f("lightPV", lightPV);
        
        glCullFace(GL_FRONT);
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
            GL_DEPTH_BUFFER_BIT,
            depthFrameBuf,
            depthMapHDL);
        glCullFace(GL_BACK);
        buffMgr.UnuseFrameBuffer(depthFrameBuf);
        
        
        buffMgr.UseFrameBuffer(mainFrameBuf);
        Shader& mainShader = shaderMgr.getShader(mainHDL);
        mainShader.use();
        mainShader.set1f("far_plane", far);
        mainShader.setMatrix4f("lightPV", lightPV);
        mainShader.set1i("depthMap", 1);
        texMgr.activateTexture(1, depthFrameBuf.depthTex);
        drawScene(Color(0.1f, 0.1f, 0.1f, 1.0f),
                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                  mainFrameBuf,
                  mainHDL,
                  camera.GetCameraPosition(),
                  camera.GetMatrix());
        drawLight(mainShader);
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
            texMgr.activateTexture(0, mainFrameBuf.getTexRef());
        } else {
            Shader& screenShader = shaderMgr.getShader(depthMapDebugHDL);
            screenShader.use();
            screenShader.set1i("texture1", 0);
            texMgr.activateTexture(0, depthFrameBuf.depthTex);
        }
        //texMgr.activateTexture(0, mainFrameBuf.depthTexID);
        quad->Draw();
        TextMgr& textmgr = TextMgr::getInstance();
        textmgr.RenderText<std::u16string>(textHDL, u"测试", 0.0f, 10.0f, 1.0f, Color(0.5, 0.8f, 0.2f));
        CheckGLError;
        textmgr.RenderText<std::string>(textHDL, "(C) LearnOpenGL.com", 540.0f, 570.0f, 1.0f, Color(0.3, 0.7f, 0.9f));
        CheckGLError;
        
    }
    
    void updateWorld() {
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
    
    void drawLight(Shader& shader) {
        shader.setMatrix4f("model", lightObj->o2w->m);
        shader.setMatrix4f("normalMat", lightObj->o2w->mInv.transpose());
        shader.setMaterial(lightObj->material);
        lightObj->Draw();
    }
    
    void drawTerrian(Shader& shader) {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        // 地面
        texMgr.activateTexture(0, terrianTex);
        texMgr.activateTexture(2, terrianNormTex);
        shader.set1i("texture1", 0);
        shader.set1i("normTex", 2);
        
        shader.setMatrix4f("model", terrian->o2w->m);
        shader.setMatrix4f("normalMat", terrian->o2w->mInv.transpose());
        assert(terrian->material);
        shader.setMaterial(terrian->material);
        terrian->Draw();
    }
    
    void drawObjs(Shader& shader, float scale = 1.0f) {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        
        texMgr.activateTexture(0, tex1);
        //texMgr.DisableTexture(2); // no normal map
        shader.set1i("texture1", 0);
        
        for(int i = 0; i < objs.size(); i++) {
            Model* obj = objs[i];
            auto oldScale = obj->scale;
            obj->SetScale(oldScale * scale);
            shader.setMatrix4f("model", obj->o2w->m);
            shader.setMatrix4f("normalMat", obj->o2w->mInv.transpose());
            assert(obj->material);
            shader.setMaterial(obj->material);
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
        CheckGLError;
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        Shader& shader = shaderMgr.getShader(hdl);
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


int main(int argc, char *argv[]) {
    // rayTraceMain(argc, argv);
    // return 0;
	MyContext context;
	context.setup(800, 600);
	context.loop();
	return 0;
}
