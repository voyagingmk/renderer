#include "stdafx.h"
#include "light.hpp"
#include <geometry.hpp>

namespace renderer {

	Light::Light() noexcept
		:softshadow(false),
		radius(0.f),
		shadowrays(0)
	{}

	Light::Light(bool s,
		float r,
		int rays) noexcept
		:softshadow(s),
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

	PointLight::PointLight(Vector3dF& p,
		bool s,
		float r,
		int rays) noexcept
		: Light(s, r, rays), pos(p)
	{}

	void PointLight::Init() {
	}
}