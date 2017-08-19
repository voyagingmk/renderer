#ifndef RENDERER_SHADERMGR_HPP
#define RENDERER_SHADERMGR_HPP

#include "base.hpp"
#include "glcommon.hpp"
#include "matrix.hpp"
#include "transform.hpp"
#include "material.hpp"
#include "light.hpp"

namespace renderer
{

enum class ShaderType
{
    Vertex = 1,
    Fragment = 2,
    Geometry = 3
};

typedef int32_t UniLoc;
typedef int32_t ShaderHDL;
typedef int32_t ShaderProgramHDL;

typedef std::map<ShaderType, std::string> ShaderFileNames;
typedef std::map<ShaderType, ShaderHDL> ShaderHDLSet;

class Shader
{
  public:
    ShaderProgramHDL hdl;
    // TODO
  public:
    Shader() : hdl(0) {}
    Shader(ShaderProgramHDL _hdl) : hdl(_hdl) {}
    virtual UniLoc getUniformLocation(const char *name);
    // by loc
    virtual void set4f(UniLoc loc, float f1, float f2, float f3, float f4);
    virtual void set3f(UniLoc loc, float f1, float f2, float f3);
    virtual void set2f(UniLoc loc, float f1, float f2);
    virtual void set1f(UniLoc loc, float f1);
    virtual void set1i(UniLoc loc, int i1);
    virtual void set1b(UniLoc loc, bool b);
    virtual void setMatrix4f(UniLoc loc, Matrix4x4 mat);
    virtual void setTransform4f(UniLoc loc, Transform4x4 trans);
    virtual void setLight(Light *light);
    // by name
    virtual void set4f(const char *name, float f1, float f2, float f3, float f4);
    virtual void set3f(const char *name, float f1, float f2, float f3);
    virtual void set3f(const char *name, Vector3dF v);
    virtual void set2f(const char *name, float f1, float f2);
    virtual void set1f(const char *name, float f1);
    virtual void set1i(const char *name, int i1);
    virtual void set1b(const char *name, bool b);
    virtual void setMatrixes4f(const char *name, std::vector<Matrix4x4> mat);
    virtual void setMatrix4f(const char *name, Matrix4x4 mat);
    virtual void setTransform4f(const char *name, Transform4x4 trans);
    virtual void use();
};
typedef std::map<ShaderProgramHDL, Shader> ShaderProgramSet;
typedef std::map<std::string, ShaderProgramHDL> ShaderProgramAlias;

class ShaderMgrBase
{
  protected:
    std::string dirpath;
    std::list<ShaderProgramHDL> spHDLs;
    ShaderProgramSet programSet;
    ShaderProgramAlias programAlias;

  public:
    virtual ~ShaderMgrBase();
    void setShaderFileDirPath(const char *path)
    {
        dirpath = std::string(path);
    }
    inline Shader &getShader(ShaderProgramHDL hdl)
    {
        return programSet[hdl];
    }
    inline Shader &getShader(const char *alias)
    {
        return getShader(std::string(alias));
    }
    inline Shader &getShader(std::string &&alias)
    {
        ShaderProgramHDL hdl = programAlias[alias];
        return programSet[hdl];
    }
    ShaderHDL loadShaderFromFile(ShaderType, const char *filename);
    virtual ShaderHDL loadShaderFromStr(ShaderType, const char *str) { return 0; }
    virtual void deleteShader(ShaderHDL shaderHDL) {}
    virtual ShaderProgramHDL createShaderProgram(ShaderFileNames) { return 0; }
    virtual ShaderProgramHDL createShaderProgram(ShaderHDLSet) { return 0; }
    virtual void deleteShaderProgram(ShaderProgramHDL) {}
    virtual void useShaderProgram(ShaderProgramHDL hdl) {}
    void setAlias(ShaderProgramHDL hdl, const char *alias)
    {
        assert(hdl);
        programAlias[alias] = hdl;
    }
    ShaderProgramHDL getShaderProgram(std::string &alias)
    {
        return programAlias[alias];
    }
    virtual bool isShader(ShaderHDL) { return false; }
    void release();
};

#ifdef USE_GL

class ShaderMgrOpenGL : public ShaderMgrBase
{
  private:
    ShaderMgrOpenGL() {}

  public:
    static ShaderMgrOpenGL &getInstance()
    {
        static ShaderMgrOpenGL mgr;
        return mgr;
    }

  public:
    virtual ~ShaderMgrOpenGL()
    {
        release();
    }
    // override
    virtual ShaderHDL loadShaderFromStr(ShaderType, const char *filename) override;
    virtual void deleteShader(ShaderHDL shaderHDL) override;
    virtual ShaderProgramHDL createShaderProgram(ShaderFileNames) override;
    virtual ShaderProgramHDL createShaderProgram(ShaderHDLSet) override;
    virtual void deleteShaderProgram(ShaderProgramHDL) override;
    virtual void useShaderProgram(ShaderProgramHDL hdl) override;
    virtual bool isShader(ShaderHDL) override;
};
typedef ShaderMgrOpenGL ShaderMgr;
#endif
};

#endif
