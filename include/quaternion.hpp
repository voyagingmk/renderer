#ifndef RENDERER_QUATERNION_HPP
#define RENDERER_QUATERNION_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "matrix.hpp"

namespace renderer {
	template<typename T>
	class Quaternion {
    public:
		float s, x, y, z;
	public:
		Quaternion():
			s(0),x(0),y(0),z(0) {}
		Quaternion(const T s, const T x, const T y, const T z):
			s(s), x(x), y(y), z(z) {}
		Quaternion(const Quaternion<T> & q) {
			s = q.s;
			x = q.x;
			y = q.y;
			z = q.z;
		}
		Quaternion<T>& operator = (const Quaternion<T> q) {
			s = q.s;
			x = q.x;
			y = q.y;
			z = q.z;
			return *this;
		}
		Quaternion<T> operator + (const Quaternion<T> q) {
			return Quaternion<T>(s + q.s, x + q.x, y + q.y, z + q.z);
		}
		Quaternion<T> operator - (const Quaternion<T> q) {
			return Quaternion<T>(s - q.s, x - q.x, y - q.y, z - q.z);
		}
		Quaternion<T> operator * (T f) {
			return Quaternion<T>(s * f, x * f, y * f, z * f);
		}
		Quaternion<T> operator * (Quaternion<T> q) {
			/*
			= (sa + xai + yaj + zak)(sb + xbi + ybj + zbk)
			= (sasb?xaxb?yayb?zazb)
			+ (saxb + sbxa + yazb?ybza)i
			+ (sayb + sbya + zaxb?zbxa)j
			+ (sazb + sbza + xayb?xbya)k
			*/
			return Quaternion<T>(
				s * q.s - x * q.x - y * q.y - z * q.z,
				s * q.x + q.s * x + y * q.z - q.y * z,
				s * q.y + q.s * y + z * q.x - q.z * x,
				s * q.z + q.s * z + x * q.y - q.x * y
 				);
		}
		Quaternion<T> operator / (T f) {
			if (f == 0)
				throw "[Quaternion /] f = 0";
			return Quaternion<T>(s / f, x / f, y / f, z / f);
		}
		Quaternion<T>& operator += (const Quaternion<T> q) {
			s += q.s; 
			x += q.x;
			y += q.y;
			z += q.z;
			return *this;
		}
		Quaternion<T>& operator -= (const Quaternion<T> q) {
			s -= q.s;
			x -= q.x;
			y -= q.y;
			z -= q.z;
			return *this;
		}
		Quaternion<T>& operator *= (T f) {
			s *= f;
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}
		Quaternion<T>& operator /= (T f) {
			s /= f;
			x /= f;
			y /= f;
			z /= f;
			return *this;
		}
		Quaternion<T> operator - () {
			return Quaternion<T>(-s, -x, -y, -z);
		}
		bool operator == (const Quaternion<T> q) {
			return s == q.s && x == q.x && y == q.y && z == q.z;
		}
        Quaternion<T>& operator *= (Quaternion<T>& q) {
            *this = (*this) * q;
            return *this;
        }
        T Norm() {
			return sqrt(s * s + x * x + y * y + z * z);
		}
		Quaternion<T> Normalize() {
			T n = Norm();
			assert(n != 0);
			T inv = T(1) / n;
			return Quaternion<T>(0, x * inv, y * inv, z * inv);
		}
		Quaternion<T> Conjugate() {
			return Quaternion<T>(s, -x, -y, -z);
		}
		Quaternion<T> Inverse() {
			T n = Norm();
			return Conjugate() / (n * n);
		}
		T Dot(const Quaternion<T>& q) {
			return s * q.s + x * q.x + y * q.y + z * q.z;
		}
		T CosTheta(const Quaternion<T>& q) {
			return Dot(q) / (Norm() * q.Norm());
		}
		Quaternion<T> Rotate(T theta, const Quaternion<T>& normAxis) {
			theta =T(0.5) * theta;
			T cosTheta = cos(theta);
			T sinTheta = sin(theta);
			Quaternion<T> rotQ(cosTheta, 
				sinTheta * normAxis.x, 
				sinTheta * normAxis.y, 
				sinTheta * normAxis.z);
			Quaternion<T> rotQInv = rotQ.Conjugate();
			return rotQ * (*this) * rotQInv;
		}
		Quaternion<T> Exp() {
			T normV = sqrt(x * x + y * y + z * z);
			T expS = exp(s);
			T s = expS * cos(normV);
			T f = expS * sin(normV) / normV;
			return Quaternion<T>(s, x * f, y * f, z * f);
		}
		Quaternion<T> Slerp() {

		}
        
        // x-axis clockwise
        static Quaternion<T> RotateX(T angle) {
            float pitchRadian = T(0.5) * Radians(angle);
            return Quaternion<T>{T(cos(pitchRadian)), T(sin(pitchRadian)), T(0.0), T(0.0)};
        }
        // y-axis clockwise
        static Quaternion<T> RotateY(T angle) {
            T yawRadian = T(0.5) * Radians(angle);
            return Quaternion<T>{T(cos(yawRadian)), T(0.0), T(sin(yawRadian)), T(0.0)};
        }
        // z-axis clockwise
        static Quaternion<T> RotateZ(T angle) {
            T rollRadian = T(0.5) * Radians(angle);
            return Quaternion<T>{T(cos(rollRadian)), T(0.0), T(0.0), T(sin(rollRadian))};
        }
        
        // *this must be normalized
        Matrix4x4 toMatrix4x4() {
            float xy = x * y;
            float yz = y * z;
            float xz = x * z;
            float xs = x * s;
            float ys = y * s;
            float zs = z * s;
            float x2 = x * x;
            float y2 = y * y;
            float z2 = z * z;
            return Matrix4x4{
                1.0f - 2.0f*(y2 + z2), 2.0f*(xy - zs), 2.0f*(xz + ys), 0.0f,
                2.0f*(xy + zs), 1.0f - 2.0f*(x2 + z2), 2.0f*(yz - xs), 0.0f,
                2.0f*(xz - ys), 2.0f*(yz + xs), 1.0f - 2.0f*(x2 + y2), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f};
        }
	};
    
    typedef Quaternion<float> QuaternionF;
}

#endif
