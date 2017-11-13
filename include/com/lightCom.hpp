#ifndef RENDERER_COM_LIGHT_HPP
#define RENDERER_COM_LIGHT_HPP

#include "base.hpp"
#include "color.hpp"
#include "com/matrix.hpp"

namespace renderer {

	class LightBase {
	public:
		LightBase(Color ambient,
			Color diffuse,
			Color specular) :
			ambient(ambient),
			diffuse(diffuse),
			specular(specular) {}
		Color ambient;
		Color diffuse;
		Color specular;
	};
	
	class PointLightCom: public LightBase {
	public:
		PointLightCom(Color ambient,
			Color diffuse,
			Color specular,
			float constant,
			float linear,
			float quadratic):
				LightBase(ambient, diffuse, specular),
				radius(0),
				constant(constant),
				linear(linear),
				quadratic(quadratic)
		{}
		float radius;
		float constant;
		float linear;
		float quadratic;
	};


	struct PointLightTransform {
		float fovy;
		float aspect;
		float n;
		float f;
		std::vector<Matrix4x4> lightPVs;
	};

	class DirLightCom : public LightBase {
	public:
		DirLightCom(Color ambient,
			Color diffuse,
			Color specular, Vector3dF dir) :
			LightBase(ambient, diffuse, specular),
			direction(dir.Normalize())
		{}
		Vector3dF direction;
	};

	class SpotLightCom : public LightBase {
	public:
		SpotLightCom(Color ambient,
			Color diffuse,
			Color specular, Vector3dF dir, RadianF cutOff, RadianF outerCutOff) :
			LightBase(ambient, diffuse, specular),
			direction(dir.Normalize()),
			cutOff(cutOff),
			outerCutOff(outerCutOff)
		{}
		Vector3dF direction;
		RadianF cutOff;
		RadianF outerCutOff;
	};

};

#endif