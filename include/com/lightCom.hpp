#ifndef RENDERER_COM_LIGHT_HPP
#define RENDERER_COM_LIGHT_HPP

#include "base.hpp"
#include "color.hpp"

namespace renderer {

	class LightBase {
	public:
		enum class Type {
			Direction = 1,
			Point = 2
		};
	};
	
	class PointLightCom: public LightBase {
	public:
		PointLightCom(Color ambient,
			Color diffuse,
			Color specular,
			float constant,
			float linear,
			float quadratic):
				radius(0),
				ambient(ambient),
				diffuse(diffuse),
				specular(specular),
				constant(constant),
				linear(linear),
				quadratic(quadratic)
		{}
		float radius;
		Color ambient;
		Color diffuse;
		Color specular;
		float constant;
		float linear;
		float quadratic;
	};
};

#endif