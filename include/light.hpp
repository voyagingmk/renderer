#ifndef RENDERER_LIGHT_HPP
#define RENDERER_LIGHT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Light {
	public:
		Light() noexcept;
		virtual void Init() = 0;
	};

	class DirectionLight: public Light {
	public:
		Vector3dF dir;
	public:
		DirectionLight(Vector3dF& dir) noexcept;
		virtual void Init();
	};

	class PointLight: public Light {
	public:
		Vector3dF pos;
	public:
		PointLight(Vector3dF& pos) noexcept;
		virtual void Init();
	};
}
#endif // RENDERER_RAY_HPP
