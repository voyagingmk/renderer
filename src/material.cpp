#include "material.hpp"
#include "geometry.hpp"
#include "color.hpp"
#include "ray.hpp"
namespace renderer {
	CheckerMaterial::CheckerMaterial(float scale, float reflectiveness) :
		Material(reflectiveness),
		scale(scale) {
	};

	PtrColor CheckerMaterial::Sample(Ray& ray, Vector& position, Vector& normal) {
		return abs((int(std::floor(position.x * 0.1)) + int(std::floor(position.z * scale))) % 2) < 1 ? Color::Black : Color::White;
	};


	PhongMaterial::PhongMaterial(PtrColor diffuse, PtrColor specular, int shininess, float reflectiveness) :
		Material(reflectiveness),
		diffuse(diffuse),
		specular(specular),
		shininess(shininess) {

	};

	PtrColor PhongMaterial::Sample(Ray& ray, Vector& position, Vector& normal) {
		float NdotL = normal.Dot(*LightDir);
		Vector&& H = (*LightDir - ray.d).Normalize();
		float NdotH = normal.Dot(H);
		Color&& diffuseTerm = *diffuse * (max(NdotL, 0.0f));
		Color&& specularTerm = *specular * (float)(std::pow(max(NdotH, 0.0f), shininess));
		Color result = Color::White->Modulate(diffuseTerm + specularTerm);
		return std::make_shared<Color>(result);
	};


	PtrVector Material::LightDir = std::make_shared<Vector>(Vector(1.f, 1.f, 1.f).Normalize());
	PtrColor Material::LightColor = Color::White;

}