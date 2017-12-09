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
    
    class MaterialSettingBase {
    public:
        MaterialSettingBase(std::string shd):
            shaderName(shd)
        {}
        virtual ~MaterialSettingBase() {}
        virtual MaterialType type() {
            return MaterialType::Unknown;
        }
        std::string shaderName;
        std::map<std::string, std::string> texList;
        
    };
    
    class MaterialPhongSettingCom: public MaterialSettingBase {
    public:
		MaterialPhongSettingCom():
            MaterialSettingBase(""),
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
         MaterialSettingBase(shd),
         ambient(a),
         diffuse(d),
         specular(s),
         reflectiveness(r),
         shininess(sh)
        {}
        virtual MaterialType type() {
            return MaterialType::Phong;
        }
        Color ambient;
        Color diffuse;
        Color specular;
        float reflectiveness;
        float shininess;
    };
    
    
    class MaterialPBRSettingCom: public MaterialSettingBase {
    public:
        MaterialPBRSettingCom(std::string shd, float r, float m):
            MaterialSettingBase(shd),
            roughness(r),
            metallic(m)
        {}
        virtual MaterialType type() {
            return MaterialType::PBR;
        }
        float roughness;
        float metallic;
    };

	class MaterialSet {
	public:
		MaterialSet():
            idCount(0) {}
        MaterialSettingID newSettingID() {
			return ++idCount;
        }
        std::map<MaterialSettingID, MaterialSettingBase*> settingDict;
		std::map< MaterialSettingAlias, MaterialSettingID> alias2id;
		MaterialSettingID idCount;
    };
        
};

#endif
