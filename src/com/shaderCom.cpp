#include "stdafx.h"
#include "com/shader.hpp"

using namespace renderer;

UniLoc Shader::getUniformLocation(const std::string& name) {
	if (locCache != nullptr) {
		auto it = locCache->find(name);
		if (it != locCache->end()) {
			return it->second;
		}
	}
	UniLoc loc = glGetUniformLocation(spHDL, name.c_str());
	if (locCache != nullptr) {
		(*locCache)[name] = loc;
		// printf("cache loc %s, %d\n", name.c_str(), (int)loc);
	}
	return loc;
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

void Shader::set1b(UniLoc loc, bool b) {
    glUniform1i(loc, b);
}

void Shader::setMatrix4f(UniLoc loc, const Matrix4x4& mat) {
    glUniformMatrix4fv(loc, 1, GL_TRUE, mat.data->data);
}

void Shader::setTransform4f(UniLoc loc, Transform4x4 trans) {
    setMatrix4f(loc, trans.GetMatrix());
}

/*
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
}*/

// by name
void Shader::set4f(const std::string& name, float f1, float f2, float f3, float f4) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set4f(loc, f1, f2, f3, f4);
}

void Shader::set3f(const std::string& name, float f1, float f2, float f3) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set3f(loc, f1, f2, f3);
}

void Shader::set3f(const std::string& name, Vector3dF v) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set3f(loc, v.x, v.y, v.z);
}

void Shader::set3f(const std::string& name, Color c) {
	UniLoc loc = getUniformLocation(name);
	if (loc == -1) {
		return;
	}
	set3f(loc, c.r(), c.g(), c.b());
}


void Shader::set2f(const std::string& name, float f1, float f2) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set2f(loc, f1, f2);
}

void Shader::set1f(const std::string& name, float f1) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set1f(loc, f1);
}

void Shader::set1i(const std::string& name, int i1) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set1i(loc, i1);
}

void Shader::set1b(const std::string& name, bool b) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    set1b(loc, b);
}

void Shader::setMatrixes4f(const std::string& name, std::vector<Matrix4x4>& mats) {
    for (GLuint i = 0; i < mats.size(); ++i){
        UniLoc loc = getUniformLocation((std::string(name) + "[" + std::to_string(i) + "]").c_str());
        if (loc == -1) {
            return;
        }
        setMatrix4f(loc, mats[i]);
    }
}
void Shader::setMatrix4f(const std::string& name, const Matrix4x4& mat) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    setMatrix4f(loc, mat);
}

void Shader::setTransform4f(const std::string& name, Transform4x4 trans) {
    UniLoc loc = getUniformLocation(name);
    if (loc == -1) {
        return;
    }
    setTransform4f(loc, trans);
}

void Shader::set3fArray(const std::string& name, std::vector<Vector3dF>& arr, int n) {
	UniLoc loc = getUniformLocation(name);
	if (loc == -1) {
		return;
	}
	if (n == 0) {
		n = arr.size();
	}
	if (n <= 0) {
		return;
	}
	for (int i = 0; i < n; i++) {
		const Vector3dF& v = arr[i];
		UniLoc loc = getUniformLocation(std::string(name) + "[" + std::to_string(i) + "]");
        assert(loc != -1);
        set3f(loc, v.x, v.y, v.z);
	}
}

void Shader::use() {
    assert(spHDL > 0);
    glUseProgram(spHDL);
}


void Shader::validate() {
	assert(spHDL > 0);
	glValidateProgram(spHDL);
}



