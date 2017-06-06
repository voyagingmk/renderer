#ifndef RENDERER_MATERIAL_HPP
#define RENDERER_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"
#include "geometry.hpp"

namespace renderer {

    enum class MaterialType {
        Base = 1,
        Phong = 2,
        Checker = 3
    };
    
	class Material {
	public:
		float reflectiveness;
	public:
		Material(float reflectiveness) :reflectiveness(reflectiveness) {};
		virtual Color Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& lightDir) = 0;
        virtual MaterialType getType() { return MaterialType::Base; }
	};

	class CheckerMaterial : public Material {
	public:
		Color color1, color2;
		float scale;
	public:
		CheckerMaterial(float scale, float reflectiveness = 0, Color c1 = Color::White, Color c2 = Color::Black);
		Color Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& lightDir);
        virtual MaterialType getType() override { return MaterialType::Checker; }
	};


	class PhongMaterial : public Material {
	public:
		Color ambient, diffuse, specular;
		int shininess;
	public:
		PhongMaterial(const Color& ambient, const Color& diffuse, const Color& specular, const int shininess, const float reflectiveness = 0);
		Color Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& lightDir);
        virtual MaterialType getType() override { return MaterialType::Phong; }
	};

	typedef std::map<int, Material*> MaterialDict;
}

#endif // RENDERER_MATERIAL_HPP
