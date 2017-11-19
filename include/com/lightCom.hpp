#ifndef RENDERER_COM_LIGHT_HPP
#define RENDERER_COM_LIGHT_HPP

#include "base.hpp"
#include "color.hpp"
#include "com/matrix.hpp"

namespace renderer {

	class LightCommon {
	public:
		LightCommon(Color ambient,
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
	
	class PointLightCom {
	public:
		PointLightCom(
			float constant,
			float linear,
			float quadratic,
			size_t shadowmapSize):
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

	class DirLightCom {
	public:
		DirLightCom(){}
	};

	struct DirLightTransform {
		float size;
		float n;
		float f;
		Matrix4x4 lightPV;
	};

	class SpotLightCom {
	public:
		SpotLightCom(Vector3dF dir,
			RadianF cutOff, 
			RadianF outerCutOff) :
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