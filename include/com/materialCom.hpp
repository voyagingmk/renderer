#ifndef RENDERER_COM_MATERIAL_HPP
#define RENDERER_COM_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"
#include "utils/defines.hpp"


namespace renderer {
    class MaterialSettingCom {
    public:
        MaterialSettingCom(Color a, Color d, Color s, float r, float sh):
         ambient(a),
         diffuse(d),
         specular(s),
         reflectiveness(r),
         shininess(sh)
        {}
        Color ambient;
        Color diffuse;
        Color specular;
        float reflectiveness;
        float shininess;
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
