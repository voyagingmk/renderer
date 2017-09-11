#ifndef RENDERER_MODEL_HPP
#define RENDERER_MODEL_HPP

#include "base.hpp"
#include "com/mesh.hpp"
#include "importer.hpp"
#include "buffermgr.hpp"
#include "shadermgr.hpp"

using namespace std;

class Model : public Shape
{
  public:
    uint8_t type;
    vector<Mesh> meshs;
  public:
    Model()
    {
        InitIdentityTransform();
    }

    virtual void Init()
    {
    }

    void CustomInit(const std::string &filename)
    {
        BufferMgr &mgr = BufferMgr::getInstance();
        LoadMesh(filename);
        mgr.CreateMeshBuffer("model" + std::to_string(id), meshs[0]);
    }

    void CustomInit(const Mesh &mesh)
    {
        BufferMgr &mgr = BufferMgr::getInstance();
        meshs.push_back(mesh);
        mgr.CreateMeshBuffer("model" + std::to_string(id), meshs[0]);
    }

    void LoadMesh(const std::string &filename)
    {
        ImporterAssimp &importer = ImporterAssimp::getInstance();
        meshs.push_back(Mesh());
        importer.Import(filename, meshs[0]);
    }

    void ReleaseMesh()
    {
    }

    void Draw(Shader *shader = nullptr)
    {
        if (shader)
        {
            shader->setMatrix4f("model", o2w->m);
            shader->setMatrix4f("normalMat", o2w->mInv.transpose());
            MaterialMgr &matMgr = MaterialMgr::getInstance();
            Material *mat = matMgr.getMaterial(matID);
            if (mat && mat->getSetting())
            {
                mat->getSetting()->uploadToShader(shader);
            }
        }
        BufferMgr &mgr = BufferMgr::getInstance();
        mgr.DrawBuffer("model" + std::to_string(id));
    }

    virtual int Intersect(Ray &, IntersectResult *)
    {
        return false;
    }

    virtual BBox Bound() const
    {
        return BBox();
    }

    virtual BBox WorldBound() const
    {
        return BBox();
    }
};

#endif
