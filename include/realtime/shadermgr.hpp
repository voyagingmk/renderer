#ifndef RENDERER_SHADERMGR_HPP
#define RENDERER_SHADERMGR_HPP

#include "base.hpp"
#include "com/glcommon.hpp"
#include "com/matrix.hpp"
#include "com/transform.hpp"
#include "com/shader.hpp"
#include "material.hpp"
#include "light.hpp"

namespace renderer
{
    
    
class ShaderMgrBase
{
  protected:
    std::string dirpath;
    SPHDLList spHDLs;
    ShaderProgramAlias programAlias;

  public:
    virtual ~ShaderMgrBase();
    void setShaderFileDirPath(const char *path)
    {
        dirpath = std::string(path);
    }
    inline Shader getShader(ShaderProgramHDL spHDL)
    {
        return Shader(spHDL);
    }
    inline Shader getShader(const char *alias)
    {
        return getShader(std::string(alias));
    }
    inline Shader getShader(std::string &&alias)
    {
        ShaderProgramHDL spHDL = programAlias[alias];
        return Shader(spHDL);
    }
    ShaderHDL loadShaderFromFile(ShaderType, const char *filename);
    virtual ShaderHDL loadShaderFromStr(ShaderType, const char *str) { return 0; }
    virtual void deleteShader(ShaderHDL shaderHDL) {}
    virtual ShaderProgramHDL createShaderProgram(ShaderFileNames) { return 0; }
    virtual ShaderProgramHDL createShaderProgram(ShaderHDLSet) { return 0; }
    virtual void deleteShaderProgram(ShaderProgramHDL) {}
    virtual void useShaderProgram(ShaderProgramHDL spHDL) {}
    void setAlias(ShaderProgramHDL spHDL, const char *alias)
    {
        assert(spHDL);
        programAlias[alias] = spHDL;
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
    virtual void useShaderProgram(ShaderProgramHDL spHDL) override;
    virtual bool isShader(ShaderHDL) override;
};
typedef ShaderMgrOpenGL ShaderMgr;
#endif
};

#endif
