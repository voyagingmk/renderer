#include "vector.hpp"
#include <algorithm>

namespace renderer {

	Vector::Vector() :x(0), y(0), z(0) {
		debug("Vector default created.");
	};

	Vector::Vector(const float x, const float y, const float z) :x(x), y(y), z(z) {
		debug("Vector param created.");
	};

	Vector::Vector(Vector && v) :x(0), y(0), z(0) {
		x = v.x;
		y = v.y;
		z = v.z;
		v.x = 0;
		v.y = 0;
		v.z = 0;
		debug("Vector move created.");
	}

	Vector::Vector(const Vector & v) {
		x = v.x;
		y = v.y;
		z = v.z;
		debug("Vector copy created.");
	}

	Vector::~Vector() {
		debug("Destroyed.");
	}

	Vector& Vector::operator = (const Vector& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		debug("Vector copy assigned.");
		return *this;
	}
	Vector& Vector::operator = (Vector&& v) {
		v.swap(*this);
		debug("Vector move assigned.");
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

	Vector Vector::cross(const Vector& v) {
		Vector && res = Vector(-z * v.y + y * v.z, z * v.x - x * v.z, -y * v.x + x * v.y);
		return res;
	}


	Vector& Vector::swap(Vector& v) {
		std::swap(*this, v);
		return *this;
	}

	float Vector::dot(const Vector& v) {
		return x * v.x + y * v.y + z * v.z;
	}

	Vector Vector::normalize() {
		float inv = 1.0f / length();
		return Vector(x * inv, y * inv, z * inv);
	}

	PtrVector Vector::Zero = std::make_shared<Vector>(0.f, 0.f, 0.f);

}