#ifndef RENDERER_LIGHT_HPP
#define RENDERER_LIGHT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Light {
	public:
		bool shadow;
		bool softshadow;
		float radius;
		int shadowrays;
	public:
		Light() noexcept;
		Light(bool s, bool ss, float r, int rays) noexcept;
		//incidence: lightPos towards targetPos
		virtual Vector3dF incidenceNormal(Vector3dF& targetPos) = 0;
		virtual Vector3dF incidence(Vector3dF& targetPos) = 0;
		virtual void Init() = 0;
	};

	class DirectionLight final: public Light {
	public:
		Vector3dF dir;
	public:
		DirectionLight(Vector3dF& dir) noexcept;
		virtual Vector3dF incidenceNormal(Vector3dF& targetPos) override {
			return dir.Normalize();
		}
		virtual Vector3dF incidence(Vector3dF& targetPos) override {
			return dir;
		}
		virtual void Init() override;
	};

	class PointLight final : public Light {
	public:
		Vector3dF pos;
	public:
		PointLight(Vector3dF& pos,
			bool shadow,
			bool softshadow,
			float radius,
			int shadowrays) noexcept;
		virtual Vector3dF incidenceNormal(Vector3dF& targetPos) override {
			return (targetPos - pos).Normalize();
		}		
		virtual Vector3dF incidence(Vector3dF& targetPos) override {
			return targetPos - pos;
		}
		virtual void Init() override;
	};

	typedef std::vector<Light*> Lights;
}
#endif // RENDERER_RAY_HPP
