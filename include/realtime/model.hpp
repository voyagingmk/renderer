#ifndef RENDERER_MODEL_HPP
#define RENDERER_MODEL_HPP

#include "base.hpp"
#include "mesh.hpp"
#include "importer.hpp"
#include "buffermgr.hpp"


using namespace std;


class Model: public Shape {
public:
    uint8_t type;
    vector<Mesh> meshs;
    static size_t modelCount;
public:
    Model() {
        modelCount += 1;
        id = modelCount;
        InitIdentityTransform();
    }

    virtual void Init() {
    }
    
    void CustomInit(const std::string& filename) {
        BufferMgrOpenGL& mgr = BufferMgrOpenGL::getInstance();
        LoadMesh(filename);
        mgr.CreateMeshBuffer("model" + std::to_string(id), meshs[0]);
    }
    
    void CustomInit(const Mesh& mesh) {
        BufferMgrOpenGL& mgr = BufferMgrOpenGL::getInstance();
        meshs.push_back(mesh);
        mgr.CreateMeshBuffer("model" + std::to_string(id), meshs[0]);
    }
    
    void LoadMesh(const std::string& filename) {
        ImporterAssimp& importer = ImporterAssimp::getInstance();
        meshs.push_back(Mesh());
        importer.Import(filename, meshs[0]);
    }

    void ReleaseMesh() {
        
    }
    
    void Draw() {
        BufferMgrOpenGL& mgr = BufferMgrOpenGL::getInstance();
        mgr.DrawBuffer("model" + std::to_string(id));
    }

    virtual int Intersect(Ray&, IntersectResult*) {
        return false;
    }
    
    virtual BBox Bound() const {
        return BBox();
    }
    
    virtual BBox WorldBound() const {
        return BBox();
    }
};

size_t Model::modelCount = 0;

#endif
