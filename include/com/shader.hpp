#ifndef RENDERER_COM_SHADER_HPP
#define RENDERER_COM_SHADER_HPP

#include "base.hpp"
#include "matrix.hpp"
#include "transform.hpp"
#include "color.hpp"

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
    typedef std::list<ShaderProgramHDL> SPHDLList;

    class Shader
    {
    public:
        ShaderProgramHDL spHDL;
        // TODO
    public:
        Shader() : spHDL(0) {}
        Shader(ShaderProgramHDL _spHDL) : spHDL(_spHDL) {}
        virtual UniLoc getUniformLocation(const char *name);
		virtual UniLoc getUniformLocation(std::string name);
        // by loc
        virtual void set4f(UniLoc loc, float f1, float f2, float f3, float f4);
        virtual void set3f(UniLoc loc, float f1, float f2, float f3);
        virtual void set2f(UniLoc loc, float f1, float f2);
        virtual void set1f(UniLoc loc, float f1);
        virtual void set1i(UniLoc loc, int i1);
        virtual void set1b(UniLoc loc, bool b);
        virtual void setMatrix4f(UniLoc loc, Matrix4x4 mat);
        virtual void setTransform4f(UniLoc loc, Transform4x4 trans);
        // virtual void setLight(Light *light);
        // by name
        virtual void set4f(const char *name, float f1, float f2, float f3, float f4);
        virtual void set3f(const char *name, float f1, float f2, float f3);
        virtual void set3f(const char *name, Vector3dF v);
		virtual void set3f(const char *name, Color c);
        virtual void set2f(const char *name, float f1, float f2);
        virtual void set1f(const char *name, float f1);
        virtual void set1i(const char *name, int i1);
        virtual void set1b(const char *name, bool b);
        virtual void setMatrixes4f(const char *name, std::vector<Matrix4x4> mat);
        virtual void setMatrix4f(const char *name, Matrix4x4 mat);
        virtual void setTransform4f(const char *name, Transform4x4 trans);
		virtual void set3fArray(const char *name, std::vector<Vector3dF>& arr, int n = 0);
        virtual void use();
    };

    typedef std::map<std::string, ShaderProgramHDL> ShaderProgramAlias;

    struct ShaderProgramSet {
        ShaderProgramAlias alias2HDL;
        SPHDLList spHDLs;
    };
};

#endif
