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

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


using namespace std;
using namespace renderer;


bool importModel( const std::string& pFile)
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
        // Normals
        p.x = aimesh->mNormals[i].x;
        p.y = aimesh->mNormals[i].y;
        p.z = aimesh->mNormals[i].z;
        v.normal = p;
        // Texture Coordinates
        if(aimesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
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
    
    bufferMgr.CreateBuffer("cube", mesh);
    
    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}



class MyContext : public RendererContextSDL {
    ShaderProgramHDL shaderProgramHDL;
    texID texID1, texID2;
public:
    MyContext():
        shaderProgramHDL(0),
        texID1(0), texID2(0) {}
	virtual void onSDLEvent(SDL_Event& e) override {
		if (e.type == SDL_QUIT) {
			shouldExit = true;
		}
	}
    virtual void onCustomSetup() override {
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        BufferMgrOpenGL& bufferMgr = BufferMgrOpenGL::getInstance();
        texMgr.setTextureDirPath("assets/images/");
        shaderMgr.setShaderFileDirPath("assets/shaders/");
        shaderProgramHDL = shaderMgr.createShaderProgram({
            { ShaderType::Vertex, "test1.vs" },
            { ShaderType::Fragment, "test1.fs"}
        });
        texID1 = texMgr.loadTexture("dog.png", "tex1");
        texID2 = texMgr.loadTexture("face.png", "tex2");

        // Uncommenting this call will result in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glViewport(0, 0, winWidth, winHeight);
        // Setup OpenGL options
        glEnable(GL_DEPTH_TEST);
        
        importModel("./assets/models/dog.obj");
        
        checkSDLError();
        checkGLError();
    }
            
    virtual void onPoll() override
    {
        ShaderMgrOpenGL& shaderMgr = ShaderMgrOpenGL::getInstance();
        TextureMgrOpenGL& texMgr = TextureMgrOpenGL::getInstance();
        BufferMgrOpenGL& bufferMgr = BufferMgrOpenGL::getInstance();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        shaderMgr.useShaderProgram(shaderProgramHDL);
        
        // Bind Textures using texture units
        texMgr.activateTexture(0, texID1);
        UniLoc loc1 = shaderMgr.getUniformLocation(shaderProgramHDL, "ourTexture1");
        shaderMgr.setUniform1i(loc1, 0);

        texMgr.activateTexture(1, texID2);
        UniLoc loc2 = shaderMgr.getUniformLocation(shaderProgramHDL, "ourTexture2");
        shaderMgr.setUniform1i(loc2, 1);
        
        Vector3dF lightPos(100.0f, 100.0f, 100.0f);
        int32_t lightPosLoc = shaderMgr.getUniformLocation(shaderProgramHDL, "lightPos");
        shaderMgr.setUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        
        UniLoc locNormal = shaderMgr.getUniformLocation(shaderProgramHDL, "normalMat");
        UniLoc locModel = shaderMgr.getUniformLocation(shaderProgramHDL, "model");
        UniLoc locView = shaderMgr.getUniformLocation(shaderProgramHDL, "view");
        UniLoc locProj = shaderMgr.getUniformLocation(shaderProgramHDL, "proj");
        
        Matrix4x4 T = Translate<Matrix4x4>({0.0f, 10.0f, -40.0f});
        Matrix4x4 S = Scale<Matrix4x4>({0.1f, 0.1f, 0.1f});
        
        const float pitch = 1.0f, yaw = 2.0f, roll = 3.0f;
        
        static QuaternionF orientation = {0.0, 0.0, 0.0, 1.0};
        QuaternionF rotX = QuaternionF::RotateX(pitch); // x
        QuaternionF rotY = QuaternionF::RotateY(yaw); // y
        QuaternionF rotZ = QuaternionF::RotateZ(roll); // z
        QuaternionF diff = rotZ * rotY * rotX;
        orientation *= diff;
        orientation = orientation.Normalize();
        Matrix4x4 R = orientation.toMatrix4x4();
        Matrix4x4 modelTrans = T * S * R;
        Matrix4x4 normalTrans = modelTrans.inverse();
        Matrix4x4 projTrans = Perspective(45.0, winWidth / (float)winHeight, 0.1, 100.0);
        Matrix4x4 viewTrans = LookAt(Vector3dF(0.0, 0.0, 1.0), Vector3dF(0.0, 0.0, -1.0), Vector3dF(0.0,1.0,0.0));

        shaderMgr.setUniformMatrix4f(locModel, modelTrans);
        shaderMgr.setUniformMatrix4f(locNormal, normalTrans);
        shaderMgr.setUniformMatrix4f(locView, viewTrans);
        shaderMgr.setUniformMatrix4f(locProj, projTrans);
        bufferMgr.DrawBuffer("cube");
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
