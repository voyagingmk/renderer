#ifndef RENDERER_COM_MATERIAL_HPP
#define RENDERER_COM_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"

struct MaterialSettingCom {
    Color ambient;
    Color diffuse;
    Color specular;
    float reflectiveness;
    float shininess;
};

struct MaterialSet {
    std::map<std::string, MaterialSettingCom*> byAlias;
	std::map<size_t, MaterialSettingCom*> byID;
};

struct MaterialCom {
	std::shared_ptr<MaterialSettingCom> setting;
};

#endif
