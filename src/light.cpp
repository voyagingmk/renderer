#include "stdafx.h"
#include "light.hpp"
#include <geometry.hpp>
#include "color.hpp"

namespace renderer {

	Light::Light() noexcept
		:shadow(false),
		softshadow(false),
		radius(0.f),
		shadowrays(0)
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

	DirectionLight::DirectionLight(Vector3dF& d) noexcept
		: dir(d)
	{}

	void DirectionLight::Init() {
	}

	Color DirectionLight::sample_L(Vector3dF& pos) {
		return Color::Black;
	};



	PointLight::PointLight(Vector3dF& p,
		bool s, 
		bool ss,
		float r,
		int rays) noexcept
		: Light(s, ss, r, rays), pos(p)
	{}

	void PointLight::Init() {
	}

	Color PointLight::sample_L(Vector3dF& pos) {
		return Color::Black;
	};
}