#include "stdafx.h"
#include "material.hpp"
#include "geometry.hpp"
#include "color.hpp"
#include "ray.hpp"

namespace renderer {
	CheckerMaterial::CheckerMaterial(float scale, float reflectiveness, Color c1, Color c2) :
		Material(reflectiveness),
		scale(scale),
		color1(c1),
		color2(c2)
	{
	};

	Color CheckerMaterial::Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& incidence) {
		return abs((int(std::floor(position.x * 0.1)) + int(std::floor(position.z * scale))) % 2) < 1 ? color1 : color2;
	};


	PhongMaterial::PhongMaterial(const Color& ambient, const Color& diffuse, const Color& specular, const int shininess, const float reflectiveness) :
		Material(reflectiveness),
        ambient(ambient),
		diffuse(diffuse),
		specular(specular),
		shininess(shininess) {

	};

	Color PhongMaterial::Sample(Ray& ray, Point3dF& position, Normal3dF& normal, Vector3dF& incidence) {
		Vector3dF lightDir = -incidence;
		float NdotL = normal.Dot(lightDir);
		Vector3dF D = lightDir - ray.d;
		if (almost_equal(D.Length(), 0.f, 2))
			return Color::White;
		Vector3dF H = D.Normalize();
		float NdotH = normal.Dot(H);
		Color diffuseTerm = diffuse * (max(NdotL, 0.0f));
		Color specularTerm = Color::Black;
		if(shininess > 0)
			specular * (float)(std::pow(max(NdotH, 0.0f), shininess));
		assert(!isnan(diffuseTerm.r()) && !isnan(diffuseTerm.r()) && !isnan(diffuseTerm.r()));
		assert(!isnan(specularTerm.r()) && !isnan(specularTerm.r()) && !isnan(specularTerm.r()));

		return Color::White.Modulate(diffuseTerm + specularTerm);
	};

}
