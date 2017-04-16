#ifndef RENDERER_SHADERMGR_HPP
#define RENDERER_SHADERMGR_HPP

#include "base.hpp"

namespace renderer {

	enum class ShaderType {
		Vertex = 1,
		Fragment = 2
	};

	typedef int ShaderHDL;
	typedef int ShaderProgramHDL;

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
		void release();
	};


#ifdef USE_GLEW

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
	};

#endif

	// using ShaderMgr = ShaderMgrOpenGL;
};

#endif