#ifndef RENDERER_MATERIAL_HPP
#define RENDERER_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"
#include "geometry.hpp"

namespace renderer {

	class Material : public std::enable_shared_from_this<Material> {
	public:
		float reflectiveness;
		static Vector3dF LightDir;
		static Color LightColor;
	public:
		Material(float reflectiveness) :reflectiveness(reflectiveness) {};
		virtual Color Sample(Ray& ray, Vector3dF& position, Vector3dF& normal) = 0;
	};

	class CheckerMaterial : public Material {
	public:
		float scale;
	public:
		CheckerMaterial(float scale, float reflectiveness = 0);
		Color Sample(Ray& ray, Vector3dF& position, Vector3dF& normal);
	};


	class PhongMaterial : public Material {
	public:
		Color diffuse, specular;
		int shininess;
	public:
		PhongMaterial(Color& diffuse, Color& specular, int shininess, float reflectiveness = 0);
		Color Sample(Ray& ray, Vector3dF& position, Vector3dF& normal);
	};
}

#endif // RENDERER_MATERIAL_HPP
