#ifndef RENDERER_COM_MATERIAL_HPP
#define RENDERER_COM_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"
#include "utils/defines.hpp"


namespace renderer {
    
    enum class MaterialType {
        Unknown,
        PBR,
        Phong
    };
    
    class MaterialSettingComBase {
    public:
        MaterialSettingComBase(std::string shd):
            shaderName(shd)
        {}
        virtual ~MaterialSettingComBase() {}
        MaterialType type() {
            return MaterialType::Unknown;
        }
        std::string shaderName;
        std::vector<std::string> texList;
        
    };
    
    class MaterialPhongSettingCom: public MaterialSettingComBase {
    public:
		MaterialPhongSettingCom():
            MaterialSettingComBase(""),
            ambient(Color::White),
			diffuse(Color::White),
			specular(Color::White),
			reflectiveness(1.0f),
			shininess(1.0f)
		{}
        MaterialPhongSettingCom(
            std::string shd,
			Color a, Color d, Color s,
			float r, float sh):
         MaterialSettingComBase(shd),
         ambient(a),
         diffuse(d),
         specular(s),
         reflectiveness(r),
         shininess(sh)
        {}
        MaterialType type() {
            return MaterialType::Phong;
        }
        Color ambient;
        Color diffuse;
        Color specular;
        float reflectiveness;
        float shininess;
    };
    
    
    class MaterialPBRSettingCom: public MaterialSettingComBase {
    public:
        MaterialPBRSettingCom(std::string shd,
            Color a, float r, float m):
            MaterialSettingComBase(shd),
            albedo(a),
            roughness(r),
            metallic(m)
        {}
        MaterialType type() {
            return MaterialType::PBR;
        }
        Color albedo;
        float roughness;
        float metallic;
    };

    struct MaterialSet {
        std::map<MaterialSettingID, MaterialSettingComBase*> settings;
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
