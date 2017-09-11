#include "stdafx.h"
#include "shadermgr.hpp"
#include "glutils.hpp"

using namespace renderer;

ShaderMgrBase::~ShaderMgrBase() {
	release();
}


ShaderHDL ShaderMgrBase::loadShaderFromFile(ShaderType type, const char* filename) {
	auto path = dirpath + std::string(filename);
	std::string source = readFile(path.c_str());
    if (source.length() == 0) {
        std::cout << "[ShaderMgr] err, loadShaderFromFile failed, wrong file:" << path << std::endl;
        return 0;
    }
	auto hdl = loadShaderFromStr(type, source.c_str());
	if(!hdl) {
		std::cout << "[ShaderMgr] err, loadShaderFromFile failed:" << path << std::endl;
	}
	return hdl;
}


void ShaderMgrBase::release() {
    std::list<ShaderHDL> spHDLsTmp = spHDLs;
	for (auto hdl: spHDLsTmp) {
		deleteShaderProgram(hdl);
	}
	spHDLs.clear();
}



#ifdef USE_GL

ShaderHDL ShaderMgrOpenGL::loadShaderFromStr(ShaderType type, const char* source) {
	const GLchar* src = source;
	GLuint shaderHDL;
	switch (type) {
	case ShaderType::Vertex:
		shaderHDL =	glCreateShader(GL_VERTEX_SHADER); 
		break;
	case ShaderType::Fragment:
		shaderHDL = glCreateShader(GL_FRAGMENT_SHADER);
		break;
        case ShaderType::Geometry:
        shaderHDL = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    default:
		return 0;
	}
	glShaderSource(shaderHDL, 1, &src, NULL);
	glCompileShader(shaderHDL);
    CheckGLError;
	// Check for compile time errors
	GLint success;
	glGetShaderiv(shaderHDL, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(shaderHDL, 512, NULL, infoLog);
		std::cout << "[ShaderMgr] err, shader compile failed: \n" << infoLog << std::endl;
		return 0;
	}
	return shaderHDL;
}

void ShaderMgrOpenGL::deleteShader(ShaderHDL shaderHDL) {
	glDeleteShader(shaderHDL);
}

ShaderProgramHDL ShaderMgrOpenGL::createShaderProgram(ShaderHDLSet shaderHDLSet) {
	for (auto pair: shaderHDLSet) {
		ShaderHDL shaderHDL = pair.second;
		if (!isShader(shaderHDL)) {
			std::cout << "[ShaderMgr] err, invalid shader hdl:" << shaderHDL << std::endl;
			return 0;
		}
	}
	GLuint hdl = glCreateProgram();
	for (auto pair : shaderHDLSet) {
		ShaderHDL shaderHDL = pair.second;
		glAttachShader(hdl, shaderHDL);
		CheckGLError;
	}
	CheckGLError;
	glLinkProgram(hdl);
	GLint success; 
	glGetProgramiv(hdl, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512]; 
		glGetProgramInfoLog(hdl, 512, NULL, infoLog);
		std::cout << "[ShaderMgr] err, shader link failed: \n" << infoLog << std::endl;
		return 0;
	}
	for (auto pair : shaderHDLSet) {
		ShaderHDL shaderHDL = pair.second;
		deleteShader(shaderHDL);
	}
	spHDLs.push_back(hdl);
	return hdl;
}

ShaderProgramHDL ShaderMgrOpenGL::createShaderProgram(ShaderFileNames names) {
	ShaderHDLSet shaderHDLSet;
	for (auto pair: names) {
        const char* path = pair.second.c_str();
        if (!path || strlen(path) == 0) {
            continue;
        }
		ShaderHDL hdl = loadShaderFromFile(pair.first, path);
		shaderHDLSet[pair.first] = hdl;
	}
	return createShaderProgram(shaderHDLSet);
}


void ShaderMgrOpenGL::deleteShaderProgram(ShaderProgramHDL hdl) {
    spHDLs.remove(hdl);
	if (glIsProgram(hdl)) {
		glDeleteProgram(hdl);
	}
    printf("[ShaderProgram released] hdl:%d\n", hdl);
}


void ShaderMgrOpenGL::useShaderProgram(ShaderProgramHDL hdl) {
	glUseProgram(hdl);
}


bool ShaderMgrOpenGL::isShader(ShaderHDL hdl) { 
	return glIsShader(hdl);
}


#endif
