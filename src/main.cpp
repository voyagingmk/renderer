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
#include "camera.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


using namespace std;
using namespace renderer;


bool importModel(const std::string& pFile, const std::string& name)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;
    
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile( pFile,
                                             aiProcess_CalcTangentSpace       |
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_SortByPType);
    
    // If the import failed, report it
    if( !scene)
    {
        cout << importer.GetErrorString() << endl;
        return false;
    }
    
    // Now we can access the file's contents.
    BufferMgrOpenGL& bufferMgr = BufferMgrOpenGL::getInstance();
    Mesh mesh;
    aiMesh* aimesh = scene->mMeshes[0];
    // Walk through each of the mesh's vertices
    for(uint32_t i = 0; i < aimesh->mNumVertices; i++)
    {
        Vertex v;
        Vector3dF p;
        p.x = aimesh->mVertices[i].x;
        p.y = aimesh->mVertices[i].y;
        p.z = aimesh->mVertices[i].z;
        v.position = p;
        if(aimesh->HasNormals()) {
            // Normals
            p.x = aimesh->mNormals[i].x;
            p.y = aimesh->mNormals[i].y;
            p.z = aimesh->mNormals[i].z;
        } else {
            p.x = p.y = p.z = 0.0;
        }
        v.normal = p;
        if(aimesh->HasVertexColors(0)){
            v.color.x = aimesh->mColors[0][i].r;
            v.color.y = aimesh->mColors[0][i].g;
            v.color.z = aimesh->mColors[0][i].b;
        } else {
            v.color = Vector3dF(0.0, 0.0, 0.0);
        }
        // Texture Coordinates
        if(aimesh->HasTextureCoords(0)) // Does the mesh contain texture coordinates?
        {
            Vector2dF uv;
            // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            uv.x = aimesh->mTextureCoords[0][i].x;
            uv.y = aimesh->mTextureCoords[0][i].y;
            v.texCoords = uv;
        }
        else
            v.texCoords = Vector2dF(0.0f, 0.0f);
         mesh.vertices.push_back(v);
    }
    // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(GLuint i = 0; i < aimesh->mNumFaces; i++)
    {
        aiFace face = aimesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for(GLuint j = 0; j < face.mNumIndices; j++)
           mesh.indexes.push_back(face.mIndices[j]);
    }
    
    bufferMgr.CreateBuffer(name, mesh);
    
    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}



class MyContext : public RendererContextSDL {
    PerspectiveCamera camera;
    ShaderProgramHDL shaderProgramHDL;
    texID texID1, texID2;
    std::map<SDL_Keycode, uint8_t> keyState;
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
        texID1 = texMgr.loadTexture("dog.png", "tex1");
        texID2 = texMgr.loadTexture("terrian.png", "tex2");

        // Uncommenting this call will result in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glViewport(0, 0, winWidth, winHeight);
        // Setup OpenGL options
        glEnable(GL_DEPTH_TEST);
        
        importModel("./assets/models/dog.obj", "dog");
        importModel("./assets/models/plane.obj", "plane");
        
        checkSDLError();
        checkGLError();
        camera.SetFov(45.0f);
        camera.SetAspect((float)winWidth / (float)winHeight);
        camera.SetNear(0.01f);
        camera.SetFar(10000.0f);
        camera.SetCameraPosition(Vector3dF(0.0f, 10.0f, 0.0f));
        
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
        updateCamera();
        draw();
    }
    
    void draw() {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        BufferMgrOpenGL& bufferMgr = BufferMgrOpenGL::getInstance();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        shaderMgr.useShaderProgram(shaderProgramHDL);
        Shader& shader = shaderMgr.getShader(shaderProgramHDL);
        
        // Bind Textures using texture units
        texMgr.activateTexture(0, texID1);
        shader.set1i("ourTexture1", 0);
        
        shader.set3f("viewPos", camera.GetCameraPosition());
        shader.set3f("material.ambient",  1.0f, 0.5f, 0.31f);
        shader.set3f("material.diffuse",  1.0f, 0.5f, 0.31f);
        shader.set3f("material.specular", 0.5f, 0.5f, 0.5f);
        shader.set1f("material.shininess", 32.0f);
        Vector3dF lightColor = Vector3dF(1.0f, 1.0f, 1.0f);
        /*
        lightColor.x = (1.0f + sin(getTimeS() * 1.0f)) * 0.5f;
        lightColor.y = (1.0f + sin(getTimeS() * 2.0f)) * 0.5f;
        lightColor.z = (1.0f + sin(getTimeS() * 1.5f)) * 0.5f;*/
        
        Vector3dF lightPos(10.0f, 15.0f, -10.0f);
        shader.set3f("light.position",  lightPos.x, lightPos.y, lightPos.z);
        shader.set3f("light.ambient",   lightColor);
        shader.set3f("light.diffuse",   lightColor);
        shader.set3f("light.specular",  1.0f, 1.0f, 1.0f);
        shader.set1f("light.constant",  1.0f);
        shader.set1f("light.linear",    0.014f);
        shader.set1f("light.quadratic", 0.0007f);
        
        Matrix4x4 T = Translate<Matrix4x4>({-10.0f, 3.0f, -30.0f});
        Matrix4x4 S = Scale<Matrix4x4>({0.1f, 0.1f, 0.1f});
        
        const float pitch = 0.0f, yaw = 1.0f, roll = 0.0f;
        
        static QuaternionF orientation = {1.0, 0.0, 0.0, 0.0};
        QuaternionF rotX = QuaternionF::RotateX(pitch); // x
        QuaternionF rotY = QuaternionF::RotateY(yaw); // y
        QuaternionF rotZ = QuaternionF::RotateZ(roll); // z
        QuaternionF diff = rotZ * rotY * rotX;
        orientation *= diff;
        orientation = orientation.Normalize();
        Matrix4x4 R = orientation.toMatrix4x4();
        // R.debug();
        Matrix4x4 modelMat = T * R * S;
        Matrix4x4 cameraMat = camera.GetMatrix();
        shader.setMatrix4f("PV", cameraMat);
     
        for(int i = 0; i < 3; i++) {
            Matrix4x4 _modelMat = modelMat;
            _modelMat = Translate<Matrix4x4>({i * 10.0f, 0.0f, 0.0f}) * _modelMat;
            shader.setMatrix4f("model", _modelMat);
            shader.setMatrix4f("normalMat", _modelMat.inverse().transpose());
            bufferMgr.DrawBuffer("dog");
        }
        
        
        
        texMgr.activateTexture(0, texID2);
        shader.set1i("ourTexture1", 0);
        shader.set3f("material.ambient",  1.0f, 1.0f, 1.0f);
        shader.set3f("material.diffuse",  1.0f, 1.0f, 1.0f);
        shader.set3f("material.specular", 1.0f, 1.0f, 1.0f);
        shader.set1f("material.shininess", 32.0f);
        modelMat = Matrix4x4::newIdentity();
        shader.setMatrix4f("model", modelMat);
        shader.setMatrix4f("normalMat", modelMat.inverse().transpose());
        bufferMgr.DrawBuffer("plane");
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
