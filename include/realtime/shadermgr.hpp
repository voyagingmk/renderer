#ifndef RENDERER_SHADERMGR_HPP
#define RENDERER_SHADERMGR_HPP

#include "base.hpp"
#include "matrix.hpp"
#include "transform.hpp"

namespace renderer {

	enum class ShaderType {
		Vertex = 1,
		Fragment = 2
	};
    
    typedef int32_t UniLoc;
	typedef int32_t ShaderHDL;
	typedef int32_t ShaderProgramHDL;

	typedef std::map<ShaderType, std::string> ShaderFileNames;
	typedef std::map<ShaderType, ShaderHDL> ShaderSet;

	class ShaderMgrBase {
	protected:
		std::string dirpath;
		std::vector<ShaderHDL> vsHDLs;
		std::vector<ShaderHDL> fsHDLs;
		std::vector<ShaderProgramHDL> spHDLs;
	public:
		virtual	~ShaderMgrBase();
		void setShaderFileDirPath(const char* path) {
			dirpath = std::string(path);
		}
		ShaderHDL loadShaderFromFile(ShaderType, const char* filename);
		virtual ShaderHDL loadShaderFromStr(ShaderType, const char* str) { return 0; }
		virtual void deleteShader(ShaderHDL shaderHDL) {}
		virtual ShaderProgramHDL createShaderProgram(ShaderFileNames) { return 0; }
		virtual ShaderProgramHDL createShaderProgram(ShaderSet) { return 0; }
		virtual void deleteShaderProgram(ShaderProgramHDL){}
		virtual void useShaderProgram(ShaderProgramHDL hdl) {}
		virtual bool isShader(ShaderHDL) { return false; }
        virtual int32_t getUniformLocation(ShaderProgramHDL, const char* name) { return 0; }
        virtual void setUniform4f(int32_t loc, float f1, float f2, float f3, float f4) {}
        virtual void setUniform3f(int32_t loc, float f1, float f2, float f3) {}
        void release();
	};


#ifdef USE_GL

	class ShaderMgrOpenGL : public ShaderMgrBase {
	private:
		ShaderMgrOpenGL() {}
	public:
		static ShaderMgrOpenGL& getInstance() {
			static ShaderMgrOpenGL mgr;
			return mgr;
		}
		// override
		virtual ShaderHDL loadShaderFromStr(ShaderType, const char* filename) override;
		virtual void deleteShader(ShaderHDL shaderHDL) override;
		virtual ShaderProgramHDL createShaderProgram(ShaderFileNames) override;
		virtual ShaderProgramHDL createShaderProgram(ShaderSet) override;
		virtual void deleteShaderProgram(ShaderProgramHDL) override;
		virtual void useShaderProgram(ShaderProgramHDL hdl) override;
		virtual bool isShader(ShaderHDL) override;
        int32_t getUniformLocation(ShaderProgramHDL, const char* name) override;
        virtual void setUniform4f(UniLoc loc, float f1, float f2, float f3, float f4) override;
        virtual void setUniform3f(int32_t loc, float f1, float f2, float f3);
        virtual void setUniform1i(UniLoc loc, int val);
        virtual void setUniformMatrix4f(UniLoc loc, Matrix4x4 mat);
        virtual void setUniformTransform4f(UniLoc loc, Transform4x4 trans);
	};

#endif

	// using ShaderMgr = ShaderMgrOpenGL;
};

#endif
