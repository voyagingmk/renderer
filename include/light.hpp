#ifndef RENDERER_LIGHT_HPP
#define RENDERER_LIGHT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Light {
	public:
		Light() noexcept;
		virtual Vector3dF incidence(Vector3dF& targetPos) = 0;
		virtual void Init() = 0;
	};

	class DirectionLight final: public Light {
	public:
		Vector3dF dir;
	public:
		DirectionLight(Vector3dF& dir) noexcept;
		virtual Vector3dF incidence(Vector3dF& targetPos) override {
			return dir;
		}
		virtual void Init() override;
	};

	class PointLight final : public Light {
	public:
		Vector3dF pos;
	public:
		PointLight(Vector3dF& pos) noexcept;
		virtual Vector3dF incidence(Vector3dF& targetPos) override {
			return (targetPos - pos).Normalize();
		}
		virtual void Init() override;
	};
}
#endif // RENDERER_RAY_HPP
