#include "geometry.hpp"
#include <algorithm>

namespace renderer {

	Point::Point() :x(0), y(0), z(0) {
		DEBUG_POINT("Point default created.");
	};

	Point::Point(const float x, const float y, const float z) :x(x), y(y), z(z) {
		DEBUG_POINT("Point param created.");
	};

	Point::Point(const Point & p) {
		x = p.x;
		y = p.y;
		z = p.z;
		DEBUG_POINT("Point copy created.");
	}

	Point::Point(Point && p) :x(0), y(0), z(0) {
		x = p.x;
		y = p.y;
		z = p.z;
		p.x = 0;
		p.y = 0;
		p.z = 0;
		DEBUG_POINT("Point move created.");
	}

	Point::~Point() {
		DEBUG_POINT("Destroyed.");
	}

	Point& Point::operator = (const Point& p) {
		x = p.x;
		y = p.y;
		z = p.z;
		DEBUG_POINT("Point copy assigned.");
		return *this;
	}

	Point Point::operator + (const Point& p) {
		return Point(x + p.x, y + p.y, z + p.z);
	}
	Point Point::operator - (const Point& p) {
		return Point(x - p.x, y - p.y, z - p.z);
	}

	Point Point::operator * (float f) {
		return Point(x * f, y * f, z * f);
	}

	Point Point::operator / (float f) {
		if (f == 0)
			throw "[Point /] f = 0";
		return Point(x / f, y / f, z / f);
	}

	Point& Point::operator += (const Point& p) {
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}

	Point& Point::operator -= (const Point& p) {
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}

	Point& Point::operator *= (float f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Point& Point::operator /= (float f) {
		x /= f;
		y /= f;
		z /= f;
		return *this;
	}

	Point Point::operator - () {
		return Point(-x, -y, -z);
	}

	bool Point::operator == (const Point& p) {
		return x == p.x && y == p.y && z == p.z;
	}

	Point Point::Cross(const Point& p) {
		return Point(-z * p.y + y * p.z, z * p.x - x * p.z, -y * p.x + x * p.y);
	}


	Point& Point::Swap(Point& p) {
		std::swap(*this, p);
		return *this;
	}

	float Point::Dot(const Point& p) {
		return x * p.x + y * p.y + z * p.z;
	}

	Point Point::Normalize() {
		float inv = 1.0f / Length();
		return Point(x * inv, y * inv, z * inv);
	}

	Point Point::Zero = Point(0.f, 0.f, 0.f);

}