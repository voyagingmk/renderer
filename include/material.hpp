#ifndef RENDERER_MATERIAL_HPP
#define RENDERER_MATERIAL_HPP

#include "base.hpp"

namespace renderer {

	class Material : public std::enable_shared_from_this<Material> {
	public:
		float reflectiveness;
		static PtrVector LightDir;
		static PtrColor LightColor;
	public:
		Material(float reflectiveness) :reflectiveness(reflectiveness) {};
		virtual PtrColor Sample(Ray& ray, Vector& position, Vector& normal) = 0;
	};

	class CheckerMaterial : public Material {
	public:
		float scale;
	public:
		CheckerMaterial(float scale, float reflectiveness = 0);
		PtrColor Sample(Ray& ray, Vector& position, Vector& normal);
	};


	class PhongMaterial : public Material {
	public:
		PtrColor diffuse, specular;
		int shininess;
	public:
		PhongMaterial(PtrColor diffuse, PtrColor specular, int shininess, float reflectiveness = 0);
		PtrColor Sample(Ray& ray, Vector& position, Vector& normal);
	};
}

#endif // RENDERER_MATERIAL_HPP
