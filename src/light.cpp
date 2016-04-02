#include "stdafx.h"
#include "light.hpp"
#include <geometry.hpp>

namespace renderer {

	Light::Light() noexcept
	{}
	void Light::Init() {
	}

	DirectionLight::DirectionLight(Vector3dF& d) noexcept
		: dir(d)
	{}
	void DirectionLight::Init() {
	}

	PointLight::PointLight(Vector3dF& p) noexcept
		: pos(p)
	{}
	void PointLight::Init() {
	}
}