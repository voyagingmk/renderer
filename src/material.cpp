#include "stdafx.h"
#include "material.hpp"
#include "geometry.hpp"
#include "color.hpp"
#include "ray.hpp"
#include "realtime/shadermgr.hpp"

namespace renderer {
    void MaterialSetting::uploadToShader(Shader *shader) {
       
    }

    void MaterialSettingPhong::uploadToShader(Shader *shader) {
        // MaterialSetting::uploadToShader(<#shader#>);
        shader->set3f("ambient", ambient.r(), ambient.g(), ambient.b());
        shader->set3f("diffuse", diffuse.r(), diffuse.g(), diffuse.b());
        shader->set3f("specular", specular.r(), specular.g(), specular.b());
        shader->set1i("shininess", shininess);
    }
    
    void MaterialSettingChecker::uploadToShader(Shader *shader) {
        
    }
    
    
    
    
    MaterialSetting* Material::getSetting() {
        return MaterialMgr::getInstance().getMaterialSetting(sID);
    }
    
	Color MaterialChecker::Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& incidence) {
        MaterialSettingChecker* s = dynamic_cast<MaterialSettingChecker*>(getSetting());
		return abs((int(std::floor(position.x * 0.1)) + int(std::floor(position.z * s->scale))) % 2) < 1 ? s->color1 : s->color2;
	};

	Color MaterialPhong::Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& incidence) {
        MaterialSettingPhong* s = dynamic_cast<MaterialSettingPhong*>(getSetting());
		Vector3dF lightDir = -incidence;
		float NdotL = normal.Dot(lightDir);
		Vector3dF D = lightDir - ray.d;
		if (almost_equal(D.Length(), 0.f, 2))
			return Color::White;
		Vector3dF H = D.Normalize();
		float NdotH = normal.Dot(H);
		Color diffuseTerm = s->diffuse * (max(NdotL, 0.0f));
		Color specularTerm = Color::Black;
		if(s->shininess > 0)
			s->specular * (float)(std::pow(max(NdotH, 0.0f), s->shininess));
		assert(!isnan(diffuseTerm.r()) && !isnan(diffuseTerm.r()) && !isnan(diffuseTerm.r()));
		assert(!isnan(specularTerm.r()) && !isnan(specularTerm.r()) && !isnan(specularTerm.r()));

		return Color::White.Modulate(diffuseTerm + specularTerm);
	};

}
