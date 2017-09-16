#ifndef RENDERER_COM_MATERIAL_HPP
#define RENDERER_COM_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"
#include "utils/defines.hpp"


namespace renderer {
    class MaterialSettingCom {
    public:
		MaterialSettingCom():
			ambient(Color::White),
			diffuse(Color::White),
			specular(Color::White),
			reflectiveness(1.0f),
			shininess(1.0f),
			shaderName("")
		{}
        MaterialSettingCom(
			Color a, Color d, Color s,
			float r, float sh, 
			std::string shd):
         ambient(a),
         diffuse(d),
         specular(s),
         reflectiveness(r),
         shininess(sh),
		 shaderName(shd)
        {}
        Color ambient;
        Color diffuse;
        Color specular;
        float reflectiveness;
        float shininess;
		std::string shaderName;
		std::vector<std::string> texList;
    };

    struct MaterialSet {
        std::map<MaterialSettingID, MaterialSettingCom> settings;
    };

    class MaterialCom {
    public:
        MaterialCom(MaterialSettingID id):
            settingID(id)
        {}
        MaterialSettingID settingID;
    };
        
};

#endif
