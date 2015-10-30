#include "geometry.hpp"
#include <algorithm>

namespace renderer {

	Point::Point() :x(0), y(0), z(0) {
		Debug("Point default created.");
	};

	Point::Point(const float x, const float y, const float z) :x(x), y(y), z(z) {
		Debug("Point param created.");
	};

	Point::Point(Point && v) :x(0), y(0), z(0) {
		x = v.x;
		y = v.y;
		z = v.z;
		v.x = 0;
		v.y = 0;
		v.z = 0;
		Debug("Point move created.");
	}

	Point::Point(const Point & v) {
		x = v.x;
		y = v.y;
		z = v.z;
		Debug("Point copy created.");
	}

	Point::~Point() {
		Debug("Destroyed.");
	}

	Point& Point::operator = (const Point& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		Debug("Point copy assigned.");
		return *this;
	}
	Point& Point::operator = (Point&& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		v.x = 0;
		v.y = 0;
		v.z = 0;
		Debug("Point move assigned.");
		return *this;
	}


	Point Point::operator + (const Point& v) {
		return Point(x + v.x, y + v.y, z + v.z);
	}
	Point Point::operator - (const Point& v) {
		return Point(x - v.x, y - v.y, z - v.z);
	}
	Point Point::operator - () {
		return Point(-x, -y, -z);
	}
	Point Point::operator * (float f) {
		return Point(x * f, y * f, z * f);
	}

	Point Point::operator / (float f) {
		if (f == 0)
			return *this;
		return Point(x / f, y / f, z / f);
	}

	bool Point::operator == (const Point& v) {
		return x == v.x && y == v.y && z == v.z;
	}

	Point Point::Cross(const Point& v) {
		Point && res = Point(-z * v.y + y * v.z, z * v.x - x * v.z, -y * v.x + x * v.y);
		return res;
	}


	Point& Point::Swap(Point& v) {
		std::swap(*this, v);
		return *this;
	}

	float Point::Dot(const Point& v) {
		return x * v.x + y * v.y + z * v.z;
	}

	Point Point::Normalize() {
		float inv = 1.0f / Length();
		return Point(x * inv, y * inv, z * inv);
	}

	Point Point::Zero = Point(0.f, 0.f, 0.f);

}