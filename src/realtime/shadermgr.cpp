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
        std::cout << "[ShaderMgr] err, loadShaderFromFile failed: wrong file\n" << std::endl;
        return 0;
    }
	return loadShaderFromStr(type, source.c_str());
}


void ShaderMgrBase::release() {
	for (auto hdl: spHDLs) {
		deleteShaderProgram(hdl);
	}
	spHDLs.clear();
	fsHDLs.clear();
	vsHDLs.clear();
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
	default:
		return 0;
	}
	glShaderSource(shaderHDL, 1, &src, NULL);
	glCompileShader(shaderHDL);
    checkGLError();
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

ShaderProgramHDL ShaderMgrOpenGL::createShaderProgram(ShaderSet shaderSet) {
	for (auto pair: shaderSet) {
		ShaderHDL shaderHDL = pair.second;
		if (!isShader(shaderHDL)) {
			std::cout << "[ShaderMgr] err, invalid shader hdl:" << shaderHDL << std::endl;
			return 0;
		}
	}
	GLuint hdl = glCreateProgram();
	for (auto pair : shaderSet) {
		ShaderHDL shaderHDL = pair.second;
		glAttachShader(hdl, shaderHDL);
		checkGLError();
	}
	checkGLError();
	glLinkProgram(hdl);
	GLint success; 
	glGetProgramiv(hdl, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512]; 
		glGetProgramInfoLog(hdl, 512, NULL, infoLog);
		std::cout << "[ShaderMgr] err, shader link failed: \n" << infoLog << std::endl;
		return 0;
	}
	for (auto pair : shaderSet) {
		ShaderHDL shaderHDL = pair.second;
		deleteShader(shaderHDL);
	}
	spHDLs.push_back(hdl);
	return hdl;
}

ShaderProgramHDL ShaderMgrOpenGL::createShaderProgram(ShaderFileNames names) {
	ShaderSet shaderSet;
	for (auto pair: names) {
		ShaderHDL hdl = loadShaderFromFile(pair.first, pair.second.c_str());
		shaderSet[pair.first] = hdl;
	}
	return createShaderProgram(shaderSet);
}


void ShaderMgrOpenGL::deleteShaderProgram(ShaderProgramHDL hdl) {
	if (glIsProgram(hdl)) {
		glDeleteProgram(hdl);
	}
}


void ShaderMgrOpenGL::useShaderProgram(ShaderProgramHDL hdl) {
	glUseProgram(hdl);
}


bool ShaderMgrOpenGL::isShader(ShaderHDL hdl) { 
	return glIsShader(hdl);
}

int32_t ShaderMgrOpenGL::getUniformLocation(ShaderProgramHDL hdl, const char* name) {
    return (int32_t)glGetUniformLocation(hdl, name);
}

void ShaderMgrOpenGL::setUniform4f(UniLoc loc, float f1, float f2, float f3, float f4) {
   glUniform4f(loc, f1, f2, f3, f4);
}

void ShaderMgrOpenGL::setUniform1i(UniLoc loc, int val) {
    glUniform1i(loc, val);
}

void ShaderMgrOpenGL::setUniformMatrix4f(UniLoc loc, Matrix4x4 mat) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat.data->data);
}


#endif
