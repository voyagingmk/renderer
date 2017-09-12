#ifndef RENDERER_MATERIAL_HPP
#define RENDERER_MATERIAL_HPP

#include "base.hpp"
#include "utils/defines.hpp"
#include "../com/color.hpp"
#include "../com/geometry.hpp"

namespace renderer {
    
    class MaterialMgr;
    class Shader;
    
    class MaterialSetting {
    public:
        float reflectiveness;
    public:
        MaterialSetting():
            reflectiveness(0)
        {}
        MaterialSetting(int r):
            reflectiveness(r)
        {}
        virtual void setDefault() = 0;
        virtual void uploadToShader(Shader* shader);
    };
    
    class MaterialSettingPhong: public MaterialSetting {
    public:
        Color ambient, diffuse, specular;
        float shininess;
    public:
        MaterialSettingPhong() {
            setDefault();
        }
        
        MaterialSettingPhong(Color a, Color d, Color s, float sh, int r):
            MaterialSetting(r),
            ambient(a),
            diffuse(d),
            specular(s),
            shininess(sh)
        {}
        
        virtual void setDefault() override {
            reflectiveness = 0;
            shininess = 1;
            specular = diffuse = ambient = Color::White;
        }
        
        virtual void uploadToShader(Shader* shader) override;
    };
    
    class MaterialSettingChecker: public MaterialSetting {
    public:
        Color color1, color2;
        float scale;
    public:
        MaterialSettingChecker() {
            setDefault();
        }
        
        MaterialSettingChecker(Color c1, Color c2, float s, int r):
            MaterialSetting(r),
            color1(c1),
            color2(c2),
            scale(s)
        {}
        
        virtual void setDefault() override {
            reflectiveness = 0;
            scale = 1;
            color1 = Color::White;
            color2 = Color::Black;
        }
        virtual void uploadToShader(Shader* shader) override;
    };

    enum class MaterialType {
        Base = 1, // normal
        Phong = 2,
        Checker = 3
    };
    
    // one Model one Material
    // use bindSetting to allow use setting commonly
	class Material {
    public:
        MaterialSettingID sID;
        Shader* shader;
	public:
        Material():
            sID(0),
            shader(nullptr)
            {};
        void bindSetting(MaterialSettingID id) {
            sID = id;
        }
        
        MaterialSetting* getSetting();
        
		virtual Color Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& lightDir) = 0;
        virtual MaterialType getType() { return MaterialType::Base; }
	};

	class MaterialChecker : public Material {
	public:
        MaterialChecker() {};
		Color Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& lightDir) override;
        virtual MaterialType getType() override { return MaterialType::Checker; }
	};


	class MaterialPhong : public Material {
	public:
        MaterialPhong() {};
		Color Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& lightDir) override;
        virtual MaterialType getType() override { return MaterialType::Phong; }
	};
    

    class MaterialMgr {
        typedef std::map<MaterialID, Material*> MaterialDict;
        typedef std::map<MaterialSettingID, MaterialSetting*> MaterialSettingDict;
        
        typedef std::map<std::string, MaterialID> MaterialAlias;
        typedef std::map<std::string, MaterialSettingID> MaterialSettingAlias;
        
        MaterialID matIDCount;
        MaterialSettingID matSettingIDCount;
    public:
        MaterialDict materials;
        MaterialSettingDict materialSettings;
        MaterialAlias materialsAlias;
        MaterialSettingAlias materialSettingAlias;
    private:
        MaterialMgr():
        matIDCount(0),
        matSettingIDCount(1)
        {}
    public:
        static MaterialMgr& getInstance() {
            static MaterialMgr mgr;
            return mgr;
        }
        
        Material* getMaterial(MaterialID matID) {
            // TODO
            return materials[matID];
        }
        
        Material* getMaterial(std::string alias) {
            return getMaterial(materialsAlias[alias]);
        }
        
        MaterialSetting* getMaterialSetting(MaterialSettingID sID) {
            // TODO
            return materialSettings[sID];
        }
        
        MaterialSetting* getMaterialSetting(std::string alias) {
            return getMaterialSetting(materialSettingAlias[alias]);
        }
        
        void setMaterialAlias(MaterialID matID, std::string alias) {
            // TODO
            materialsAlias[alias] = matID;
        }
        
        void setMaterialSettingAlias(MaterialSettingID sID, std::string alias) {
            // TODO
            materialSettingAlias[alias] = sID;
        }
        
        MaterialID addMaterial(Material* mat) {
            matIDCount++;
            MaterialID matID = matIDCount;
            materials[matID] = mat;
            return matID;
        }
        
        
        MaterialID addMaterialSetting(MaterialSetting* s) {
            matSettingIDCount++;
            MaterialID sID = matSettingIDCount;
            materialSettings[sID] = s;
            return sID;
        }
        
    };
}

#endif // RENDERER_MATERIAL_HPP
