#include "geometry.hpp"
#include <algorithm>

namespace renderer {

	Vector::Vector() :x(0), y(0), z(0) {
		Debug("Vector default created.");
	};

	Vector::Vector(const float x, const float y, const float z) :x(x), y(y), z(z) {
		Debug("Vector param created.");
	};

	Vector::Vector(Vector && v) :x(0), y(0), z(0) {
		x = v.x;
		y = v.y;
		z = v.z;
		v.x = 0;
		v.y = 0;
		v.z = 0;
		Debug("Vector move created.");
	}

	Vector::Vector(const Vector & v) {
		x = v.x;
		y = v.y;
		z = v.z;
		Debug("Vector copy created.");
	}

	Vector::~Vector() {
		Debug("Destroyed.");
	}

	Vector& Vector::operator = (const Vector& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		Debug("Vector copy assigned.");
		return *this;
	}
	Vector& Vector::operator = (Vector&& v) {
		v.Swap(*this);
		Debug("Vector move assigned.");
		return *this;
	}


	Vector Vector::operator + (const Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	Vector Vector::operator - (const Vector& v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	Vector Vector::operator - () {
		return Vector(-x, -y, -z);
	}
	Vector Vector::operator * (float f) {
		return Vector(x * f, y * f, z * f);
	}

	Vector Vector::operator / (float f) {
		if (f == 0)
			return *this;
		return Vector(x / f, y / f, z / f);
	}

	bool Vector::operator == (const Vector& v) {
		return x == v.x && y == v.y && z == v.z;
	}

	Vector Vector::Cross(const Vector& v) {
		Vector && res = Vector(-z * v.y + y * v.z, z * v.x - x * v.z, -y * v.x + x * v.y);
		return res;
	}


	Vector& Vector::Swap(Vector& v) {
		std::swap(*this, v);
		return *this;
	}

	float Vector::Dot(const Vector& v) {
		return x * v.x + y * v.y + z * v.z;
	}

	Vector Vector::Normalize() {
		float inv = 1.0f / Length();
		return Vector(x * inv, y * inv, z * inv);
	}

	PtrVector Vector::Zero = std::make_shared<Vector>(0.f, 0.f, 0.f);

}