#include "stdafx.h"
#include "material.hpp"
#include "geometry.hpp"
#include "color.hpp"
#include "ray.hpp"

namespace renderer {
	CheckerMaterial::CheckerMaterial(float scale, float reflectiveness) :
		Material(reflectiveness),
		scale(scale) {
	};

	Color CheckerMaterial::Sample(Ray& ray, Vector3dF& position, Vector3dF& normal) {
		return abs((int(std::floor(position.x * 0.1)) + int(std::floor(position.z * scale))) % 2) < 1 ? Color::Black : Color::White;
	};


	PhongMaterial::PhongMaterial(Color& diffuse, Color& specular, int shininess, float reflectiveness) :
		Material(reflectiveness),
		diffuse(diffuse),
		specular(specular),
		shininess(shininess) {

	};

	Color PhongMaterial::Sample(Ray& ray, Vector3dF& position, Vector3dF& normal) {
		float NdotL = normal.Dot(LightDir);
		Vector3dF H = (LightDir - ray.d).Normalize();
		float NdotH = normal.Dot(H);
		Color diffuseTerm = diffuse * (max(NdotL, 0.0f));
		Color specularTerm = specular * (float)(std::pow(max(NdotH, 0.0f), shininess));
		return Color::White.Modulate(diffuseTerm + specularTerm);
	};


	Vector3dF Material::LightDir = Vector3dF(1.f, 1.f, 1.f).Normalize();
	Color Material::LightColor = Color::White;

}