#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "base.hpp"

namespace renderer {

	class Material : public std::enable_shared_from_this<Material> {
	protected:
		float m_reflectiveness;
	public:
		Material(float reflectiveness) :m_reflectiveness(reflectiveness) {};
		virtual PtrColor sample(PtrRay ray, PtrVector position, PtrVector normal) = 0;
		inline float getReflectiveness() { return m_reflectiveness; };
	public:
		static PtrVector LightDir;
		static PtrColor LightColor;
	};

	class CheckerMaterial : public Material {
		float m_scale;
	public:
		CheckerMaterial(float scale, float reflectiveness = 0);
		PtrColor sample(PtrRay ray, PtrVector position, PtrVector normal);
	};


	class PhongMaterial : public Material {
		PtrColor m_diffuse, m_specular;
		int m_shininess;
	public:
		PhongMaterial(PtrColor diffuse, PtrColor specular, int shininess, float reflectiveness = 0);
		PtrColor sample(PtrRay ray, PtrVector position, PtrVector normal);
		inline PtrColor getDiffuse() { return m_diffuse; };
		inline PtrColor getSpecular() { return m_specular; };
	};
}

#endif // MATERIAL_HPP
