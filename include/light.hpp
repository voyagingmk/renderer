#ifndef RENDERER_LIGHT_HPP
#define RENDERER_LIGHT_HPP

#include "base.hpp"
#include "com/geometry.hpp"
#include "color.hpp"

namespace renderer {
    enum class LightType {
        Default = 0,
        Direction = 1,
        Point = 2
    };

	class Light {
	public:
		bool shadow;
		bool softshadow;
		float radius;
		int shadowrays;
        Color ambient;
        Color diffuse;
        Color specular;
        float constant;
        float linear;
        float quadratic;
	public:
		Light() noexcept;
		Light(bool s, bool ss, float r = 1.0f, int rays = 0) noexcept;
		//incidence: lightPos towards targetPos
		virtual Normal3dF incidenceNormal(Point3dF& targetPos) = 0;
		virtual Vector3dF incidence(Point3dF& targetPos) = 0;
		virtual Color sample_L(Vector3dF& pos) = 0;
		virtual void Init() = 0;
        virtual LightType getType() { return LightType::Default; }
	};

	class DirectionLight final: public Light {
	public:
		Vector3dF dir;
	public:
		DirectionLight(bool s, bool ss, const Vector3dF& dir) noexcept;
        virtual LightType getType() override { return LightType::Direction; }
		virtual Normal3dF incidenceNormal(Point3dF& targetPos) override {
			return dir.Normalize();
		}
		virtual Vector3dF incidence(Point3dF& targetPos) override {
			return dir;
		}
		virtual Color sample_L(Vector3dF& pos) override;
		virtual void Init() override;
	};

	class PointLight final : public Light {
	public:
		Vector3dF pos;
	public:
        PointLight(const Vector3dF& pos,
            const float radius = 1.0f,
			const bool shadow = false,
			const bool softshadow = false,
			const int shadowrays = 0) noexcept;
        virtual LightType getType() override { return LightType::Point; }
		virtual Normal3dF incidenceNormal(Point3dF& targetPos) override {
			return (targetPos - pos).Normalize();
		}		
		virtual Vector3dF incidence(Point3dF& targetPos) override {
			return targetPos - pos;
		}
		virtual Color sample_L(Vector3dF& pos) override;
		virtual void Init() override;
	};

	typedef std::vector<Light*> Lights;
}
#endif // RENDERER_RAY_HPP
