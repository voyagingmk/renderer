#include "material.hpp"
#include "vector.hpp"
#include "color.hpp"
#include "ray.hpp"
namespace renderer {
	CheckerMaterial::CheckerMaterial(float scale, float reflectiveness) :
		Material(reflectiveness),
		m_scale(scale) {
	};

	PtrColor CheckerMaterial::sample(PtrRay ray, PtrVector position, PtrVector normal) {
		return abs((int(std::floor(position->x() * 0.1)) + int(std::floor(position->z() * m_scale))) % 2) < 1 ? Color::Black : Color::White;
	};


	PhongMaterial::PhongMaterial(PtrColor diffuse, PtrColor specular, int shininess, float reflectiveness) :
		Material(reflectiveness),
		m_diffuse(diffuse),
		m_specular(specular),
		m_shininess(shininess) {

	};

	PtrColor PhongMaterial::sample(PtrRay ray, PtrVector position, PtrVector normal) {
		float NdotL = normal->dot(*LightDir);
		Vector&& H = (*LightDir - *(ray->getDirection())).normalize();
		float NdotH = normal->dot(H);
		Color&& diffuseTerm = *m_diffuse * (max(NdotL, 0.0f));
		Color&& specularTerm = *m_specular * (float)(std::pow(max(NdotH, 0.0f), m_shininess));
		Color result = Color::White->modulate(diffuseTerm + specularTerm);
		return std::make_shared<Color>(result);
	};


	PtrVector Material::LightDir = std::make_shared<Vector>(Vector(1.f, 1.f, 1.f).normalize());
	PtrColor Material::LightColor = Color::White;

}