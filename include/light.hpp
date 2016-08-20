#ifndef RENDERER_LIGHT_HPP
#define RENDERER_LIGHT_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	constexpr int LightType_Default = 0;
	constexpr int LightType_Direction = 1;
	constexpr int LightType_Point = 2;

	class Light {
	public:
		bool shadow;
		bool softshadow;
		float radius;
		int shadowrays;
		int lightType = LightType_Default;
	public:
		Light() noexcept;
		Light(bool s, bool ss, float r, int rays) noexcept;
		//incidence: lightPos towards targetPos
		virtual Normal3dF incidenceNormal(Point3dF& targetPos) = 0;
		virtual Vector3dF incidence(Point3dF& targetPos) = 0;
		virtual void Init() = 0;
	};

	class DirectionLight final: public Light {
	public:
		Vector3dF dir;
	public:
		DirectionLight(Vector3dF& dir) noexcept;
		virtual Normal3dF incidenceNormal(Point3dF& targetPos) override {
			return dir.Normalize();
		}
		virtual Vector3dF incidence(Point3dF& targetPos) override {
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
		virtual Normal3dF incidenceNormal(Point3dF& targetPos) override {
			return (targetPos - pos).Normalize();
		}		
		virtual Vector3dF incidence(Point3dF& targetPos) override {
			return targetPos - pos;
		}
		virtual void Init() override;
	};

	typedef std::vector<Light*> Lights;
}
#endif // RENDERER_RAY_HPP
