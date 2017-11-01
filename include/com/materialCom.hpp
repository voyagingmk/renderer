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
        virtual MaterialType type() {
            return MaterialType::Unknown;
        }
        std::string shaderName;
        std::vector<std::tuple<std::string, std::string>> texList;
        
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
        virtual MaterialType type() {
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
        MaterialPBRSettingCom(std::string shd, float r, float m):
            MaterialSettingComBase(shd),
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
        std::map<MaterialSettingID, MaterialSettingComBase*> settings;
		std::map< MaterialSettingAlias, MaterialSettingID> alias2id;
		MaterialSettingID idCount;
    };

    class MaterialCom {
	public:
		std::vector<MaterialSettingID> settingIDs;
    };
        
};

#endif
