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
			Color specular,
			float intensity) :
			ambient(ambient),
			diffuse(diffuse),
			specular(specular),
			intensity(intensity) {}
		Color ambient;
		Color diffuse;
		Color specular;
		float intensity;
	};
	
	class PointLightCom: public LightBase {
	public:
		PointLightCom(Color ambient,
			Color diffuse,
			Color specular,
			float intensity,
			float constant,
			float linear,
			float quadratic,
			size_t shadowmapSize):
				LightBase(ambient, diffuse, specular, intensity),
				radius(0),
				constant(constant),
				linear(linear),
				quadratic(quadratic),
				shadowmapSize(shadowmapSize)
		{}
		float radius;
		float constant;
		float linear;
		float quadratic;
		size_t shadowmapSize;
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
			Color specular,
			float intensity,
			Vector3dF dir) :
			LightBase(ambient, diffuse, specular, intensity),
			direction(dir.Normalize())
		{}
		Vector3dF direction;
	};

	class SpotLightCom : public LightBase {
	public:
		SpotLightCom(Color ambient,
			Color diffuse,
			Color specular,
			float intensity,
			Vector3dF dir,
			RadianF cutOff, 
			RadianF outerCutOff) :
			LightBase(ambient, diffuse, specular, intensity),
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