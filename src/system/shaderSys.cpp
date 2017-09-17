#include "stdafx.h"
#include "com/spatialData.hpp"
#include "system/shaderSys.hpp"
#include "realtime/glutils.hpp"
#include "com/cameraCom.hpp"

using namespace std;
 

namespace renderer {
    void ShaderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("ShaderSystem init\n");
		evtMgr.on<LoadShaderEvent>(*this);
		evtMgr.on<UploadMatrixToShaderEvent>(*this);
		evtMgr.on<UploadCameraToShaderEvent>(*this);
	}

    
    void ShaderSystem::receive(const LoadShaderEvent& evt) {
        auto com = m_objMgr->getSingletonComponent<ShaderProgramSet>();

        const ShaderFileNames& names = evt.names;
        ShaderHDLSet shaderHDLSet;
        for (auto pair: names) {
            const char* path = pair.second.c_str();
            if (!path || strlen(path) == 0) {
                continue;
            }
            ShaderHDL hdl = loadShaderFromFile(pair.first, (evt.dirpath + path).c_str());
            shaderHDLSet[pair.first] = hdl;
        }
        auto spHDL = createShaderProgram(com->spHDLs, shaderHDLSet);
        assert(spHDL);
        if (!spHDL)
        {
            return;
        }
        com->alias2HDL[evt.aliasname] = spHDL;
        printf("ShaderSystem load %s\n", evt.aliasname.c_str());
    }

	void ShaderSystem::receive(const UploadMatrixToShaderEvent& evt) {
		auto com = evt.obj.component<SpatialData>();
		Shader& shader = const_cast<Shader&>(evt.shader);
		shader.setMatrix4f("model", com->o2w.m);
		shader.setMatrix4f("normalMat", com->o2w.mInv.transpose());
	}

	void ShaderSystem::receive(const UploadCameraToShaderEvent& evt) {
		Object objCamera = evt.objCamera;
		auto com = objCamera.component<PerspectiveCameraView>();
		Shader& shader = const_cast<Shader&>(evt.shader);
		auto viewMat = LookAt(com->GetCameraPosition(), com->GetTargetVector(), com->GetUpVector());
		auto projMat = Perspective(com->fov, com->aspect, com->near, com->far);
		auto cameraMat = projMat * viewMat;
		shader.set3f("viewPos", com->eye);
		shader.setMatrix4f("PV", cameraMat);
	}
    
    ShaderProgramHDL ShaderSystem::createShaderProgram(SPHDLList& spHDLs, ShaderHDLSet shaderHDLSet) {
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
    
    ShaderHDL ShaderSystem::loadShaderFromFile(ShaderType type, const char* path) {
        ShaderHDL shaderHDL = 0;
        std::string source = readFile(path);
        if (source.length() == 0) {
            std::cout << "[ShaderMgr] err, loadShaderFromFile failed, wrong file:" << path << std::endl;
            return shaderHDL;
        }
        shaderHDL = loadShaderFromStr(type, source.c_str());
        if(!shaderHDL) {
            std::cout << "[ShaderMgr] err, loadShaderFromFile failed:" << path << std::endl;
        }
        return shaderHDL;
    }
    
    ShaderHDL ShaderSystem::loadShaderFromStr(ShaderType type, const char* source) {
        const GLchar* src = source;
        ShaderHDL shaderHDL;
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
    
    void ShaderSystem::deleteShader(ShaderHDL shaderHDL) {
        glDeleteShader(shaderHDL);
    }
    
    bool ShaderSystem::isShader(ShaderHDL hdl) {
        return glIsShader(hdl);
    }
    /*
    void ShaderSystem::release() {
        std::list<ShaderHDL> spHDLsTmp = spHDLs;
        for (auto hdl: spHDLsTmp) {
            deleteShaderProgram(hdl);
        }
        spHDLs.clear();
    }*/
    
};
