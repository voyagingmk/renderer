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
	typedef std::map<std::string, UniLoc> UniformLocCache;
	typedef std::map<ShaderProgramHDL, UniformLocCache> SPHDL2UniformLocCache;

    class Shader
    {
    public:
        ShaderProgramHDL spHDL;
		UniformLocCache* locCache;
        // TODO
    public:
        Shader() : 
			spHDL(0),
			locCache(nullptr)
		{}

		Shader(ShaderProgramHDL _spHDL) :
			spHDL(_spHDL),
			locCache(nullptr)
		{}

        Shader(ShaderProgramHDL _spHDL, SPHDL2UniformLocCache& spHDL2locCache) :
			spHDL(_spHDL),
			locCache(&spHDL2locCache[_spHDL])
		{ 
			assert(spHDL); 
		}
		UniLoc getUniformLocation(const std::string& name);
        // by loc
        void set4f(UniLoc loc, float f1, float f2, float f3, float f4);
        void set3f(UniLoc loc, float f1, float f2, float f3);
        void set2f(UniLoc loc, float f1, float f2);
        void set1f(UniLoc loc, float f1);
        void set1i(UniLoc loc, int i1);
        void set1b(UniLoc loc, bool b);
        void setMatrix4f(UniLoc loc, const Matrix4x4&);
        void setTransform4f(UniLoc loc, Transform4x4 trans);
        // void setLight(Light *light);
        // by name
        void set4f(const std::string& name, float f1, float f2, float f3, float f4);
        void set3f(const std::string& name, float f1, float f2, float f3);
        void set3f(const std::string& name, Vector3dF v);
		void set3f(const std::string& name, Color c);
        void set2f(const std::string& name, float f1, float f2);
        void set1f(const std::string& name, float f1);
        void set1i(const std::string& name, int i1);
        void set1b(const std::string& name, bool b);
        void setMatrixes4f(const std::string& name, std::vector<Matrix4x4>& mat);
        void setMatrix4f(const std::string& name, const Matrix4x4&);
        void setTransform4f(const std::string& name, Transform4x4 trans);
		void set3fArray(const std::string& name, std::vector<Vector3dF>& arr, int n = 0);
        void use();
		void validate();
    };

    typedef std::map<std::string, ShaderProgramHDL> ShaderProgramAlias;

    struct ShaderProgramSet {
        ShaderProgramAlias alias2HDL;
        SPHDLList spHDLs;
		SPHDL2UniformLocCache spHDL2locCache;
    };
};

#endif
