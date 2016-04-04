#ifndef RENDERER_MATERIAL_HPP
#define RENDERER_MATERIAL_HPP

#include "base.hpp"
#include "color.hpp"
#include "geometry.hpp"

namespace renderer {

	class Material {
	public:
		float reflectiveness;
	public:
		Material(float reflectiveness) :reflectiveness(reflectiveness) {};
		virtual Color Sample(Ray& ray, Vector3dF& position, Vector3dF& normal, Vector3dF& lightDir) = 0;
	};

	class CheckerMaterial : public Material {
	public:
		Color color1, color2;
		float scale;
	public:
		CheckerMaterial(float scale, float reflectiveness = 0, Color c1 = Color::White, Color c2 = Color::Black);
		Color Sample(Ray& ray, Vector3dF& position, Vector3dF& normal, Vector3dF& lightDir);
	};


	class PhongMaterial : public Material {
	public:
		Color diffuse, specular;
		int shininess;
	public:
		PhongMaterial(Color& diffuse, Color& specular, int shininess, float reflectiveness = 0);
		Color Sample(Ray& ray, Vector3dF& position, Vector3dF& normal, Vector3dF& lightDir);
	};
}

#endif // RENDERER_MATERIAL_HPP
