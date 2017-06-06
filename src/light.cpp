#include "stdafx.h"
#include "light.hpp"
#include "geometry.hpp"
#include "color.hpp"

namespace renderer {

	Light::Light() noexcept
		:shadow(false),
		softshadow(false),
		radius(0.f),
		shadowrays(0),
        ambient(1.0f, 1.0f, 1.0f),
        diffuse(1.0f, 1.0f, 1.0f),
        specular(1.0f, 1.0f, 1.0f),
        constant(1.0),
        linear(0.0),
        quadratic(0.0)
	{}

	Light::Light(bool s,
		bool ss,
		float r,
		int rays) noexcept
		:shadow(s),
		softshadow(ss),
		radius(r),
		shadowrays(rays)
	{}

	void Light::Init() {
	}

	DirectionLight::DirectionLight(const Vector3dF& d) noexcept
		: dir(d)
	{}

	void DirectionLight::Init() {
	}

	Color DirectionLight::sample_L(Vector3dF& pos) {
		return Color::Black;
	};



	PointLight::PointLight(const Vector3dF& p,
		const float r,
		const bool s,
		const bool ss,
		const int rays) noexcept
		: Light(s, ss, r, rays), pos(p)
	{}

	void PointLight::Init() {
	}

	Color PointLight::sample_L(Vector3dF& pos) {
		return Color::Black;
	};
}
