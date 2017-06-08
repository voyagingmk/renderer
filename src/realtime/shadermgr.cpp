#include "stdafx.h"
#include "shadermgr.hpp"
#include "glutils.hpp"

using namespace renderer;


UniLoc Shader::getUniformLocation(const char* name) {
     return (UniLoc)glGetUniformLocation(hdl, name);
}

void Shader::set4f(UniLoc loc, float f1, float f2, float f3, float f4) {
    glUniform4f(loc, f1, f2, f3, f4);
}

void Shader::set3f(UniLoc loc, float f1, float f2, float f3) {
    glUniform3f(loc, f1, f2, f3);
}

void Shader::set2f(UniLoc loc, float f1, float f2) {
     glUniform2f(loc, f1, f2);
}

void Shader::set1f(UniLoc loc, float f1) {
     glUniform1f(loc, f1);
}

void Shader::set1i(UniLoc loc, int i1) {
     glUniform1i(loc, i1);
}

void Shader::setMatrix4f(UniLoc loc, Matrix4x4 mat) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat.transpose().data->data);
}

void Shader::setTransform4f(UniLoc loc, Transform4x4 trans) {
    setMatrix4f(loc, trans.GetMatrix());
}


void Shader::setLight(Light* light) {
    set3f("light.ambient", light->ambient.r(), light->ambient.g(), light->ambient.b());
    set3f("light.diffuse", light->diffuse.r(), light->diffuse.g(), light->diffuse.b());
    set3f("light.specular", light->specular.r(), light->specular.g(), light->specular.b());
    set1f("light.constant",  light->constant);
    set1f("light.linear",    light->linear);
    set1f("light.quadratic", light->quadratic);
    if(light->getType() == LightType::Point) {
        PointLight* pl = (PointLight*)light;
        set3f("light.position",  pl->pos);
    }
}

void Shader::setMaterial(Material* mat) {
    if(mat->getType() == MaterialType::Phong) {
        PhongMaterial * pmat = (PhongMaterial*)mat;
        set3f("material.ambient", pmat->ambient.r(), pmat->ambient.g(), pmat->ambient.b());
        set3f("material.diffuse", pmat->diffuse.r(), pmat->diffuse.g(), pmat->diffuse.b());
        set3f("material.specular", pmat->specular.r(), pmat->specular.g(), pmat->specular.b());
        set1f("material.shininess", pmat->shininess);
    }
}


// by name
void Shader::set4f(const char* name, float f1, float f2, float f3, float f4) {
    set4f(getUniformLocation(name), f1, f2, f3, f4);
}

void Shader::set3f(const char* name, float f1, float f2, float f3) {
    set3f(getUniformLocation(name), f1, f2, f3);
}

void Shader::set3f(const char* name, Vector3dF v) {
    set3f(getUniformLocation(name), v.x, v.y, v.z);
}


void Shader::set2f(const char* name, float f1, float f2) {
    set2f(getUniformLocation(name), f1, f2);
}

void Shader::set1f(const char* name, float f1) {
    set1f(getUniformLocation(name), f1);
}

void Shader::set1i(const char* name, int i1) {
    set1i(getUniformLocation(name), i1);
}

void Shader::setMatrix4f(const char* name, Matrix4x4 mat) {
    setMatrix4f(getUniformLocation(name), mat);
}

void Shader::setTransform4f(const char* name, Transform4x4 trans) {
    setTransform4f(getUniformLocation(name), trans);
}

void Shader::use() {
    glUseProgram(hdl);
}


ShaderMgrBase::~ShaderMgrBase() {
	release();
}


ShaderHDL ShaderMgrBase::loadShaderFromFile(ShaderType type, const char* filename) {
	auto path = dirpath + std::string(filename);
	std::string source = readFile(path.c_str());
    if (source.length() == 0) {
        std::cout << "[ShaderMgr] err, loadShaderFromFile failed: wrong file." << std::endl;
        return 0;
    }
	auto hdl = loadShaderFromStr(type, source.c_str());
	if(!hdl) {
		std::cout << "[ShaderMgr] err, loadShaderFromFile failed:" << filename << std::endl;
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
	for (auto pair : shaderHDLSet) {
		ShaderHDL shaderHDL = pair.second;
		deleteShader(shaderHDL);
	}
	spHDLs.push_back(hdl);
    programSet.insert({hdl, Shader(hdl)});
	return hdl;
}

ShaderProgramHDL ShaderMgrOpenGL::createShaderProgram(ShaderFileNames names) {
	ShaderHDLSet shaderHDLSet;
	for (auto pair: names) {
		ShaderHDL hdl = loadShaderFromFile(pair.first, pair.second.c_str());
		shaderHDLSet[pair.first] = hdl;
	}
	return createShaderProgram(shaderHDLSet);
}


void ShaderMgrOpenGL::deleteShaderProgram(ShaderProgramHDL hdl) {
    auto it = programSet.find(hdl);
    if(it == programSet.end()) {
        return;
    }
    programSet.erase(it);
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
