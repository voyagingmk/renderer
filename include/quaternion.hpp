#ifndef RENDERER_QUATERNION_HPP
#define RENDERER_QUATERNION_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	template<typename T>
	class Quaternion {
		float s, x, y, z;
	public:
		Quaternion():
			s(0),x(0),y(0),z(0) {}
		Quaternion(const T s, const T x, const T y, const T z):
			s(s), x(x), y(y), z(z) {}
		Quaternion(const Quaternion<T> & p) {
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
		T Norm() {
			return sqrt(s * s, x * x, y * y, z * z);
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
			Quaternion<T> rotQInv(rotQ.s, -rotQ.x, -rotQ.y, -rotQ.z);
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
	};
}

#endif