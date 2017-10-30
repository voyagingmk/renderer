#ifndef RENDERER_REALTIME_IMPORTER_HPP
#define RENDERER_REALTIME_IMPORTER_HPP

#include "base.hpp"
#include "com/geometry.hpp"
#include "com/meshes.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

using namespace renderer;
using namespace std;

class ImporterBase {
protected:
    ImporterBase() {}
public:
    static ImporterBase& getInstance() {
        static ImporterBase mgr;
        return mgr;
    }
    void Import() {
        
    }
};


class ImporterAssimp: public ImporterBase {
protected:
    ImporterAssimp() {}
public:
    
    static ImporterAssimp& getInstance() {
        static ImporterAssimp mgr;
        return mgr;
    }
    
    
    bool Import(const std::string& pFile, Meshes& com, bool normalInverse = false) {
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
        for (int i = 0; i < scene->mNumMeshes; i++)
        {
            aiMesh* aimesh = scene->mMeshes[i];
            com.meshes.push_back(OneMesh());
            OneMesh& mesh = com.meshes[com.meshes.size() - 1];
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
					if (normalInverse) {
						p = -p;
					}
                } else {
                    p.x = p.y = p.z = 0.0;
                }
                v.normal = p;
				
				Vector3dF tangent;
				Vector3dF bitangent;
				if (aimesh->HasTangentsAndBitangents()) {
					v.tangent.x = aimesh->mTangents[i].x;
					v.tangent.y = aimesh->mTangents[i].y;
					v.tangent.z = aimesh->mTangents[i].z;
					v.bitangent.x = aimesh->mTangents[i].x;
					v.bitangent.y = aimesh->mTangents[i].y;
					v.bitangent.z = aimesh->mTangents[i].z;
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
        }
        // We're done. Everything will be cleaned up by the importer destructor
        return true;
    }
};


#endif
