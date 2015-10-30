#ifndef RENDERER_GEOMETRY_HPP
#define RENDERER_GEOMETRY_HPP

#include "base.hpp"


namespace renderer {

	class Point {
	public:
		float x, y, z;
		static Point Zero;
	public:
		Point();
		Point(const float x, const float y, const float z);
		Point(const Point &);
		Point(Point &&);
		Point& operator = (const Point&);
		~Point();
		Point operator + (const Point&);
		Point operator - (const Point&);
		Point operator * (float f);
		Point operator / (float f);
		Point& operator += (const Point&);
		Point& operator -= (const Point&);
		Point& operator *= (float f);
		Point& operator /= (float f);
		Point operator - ();
		bool operator == (const Point& v);
		float operator[](int i) const {
			return (&x)[i];
		}
		float &operator[](int i) {
			return (&x)[i];
		}
		float Dot(const Point&);
		Point Cross(const Point&);
		inline float Length() { return sqrt(x * x + y * y + z * z); };
		inline float LengthSquare() { return x * x + y * y + z * z; };
		Point Normalize();
		Point& Swap(Point&);
	};

}

#endif // RENDERER_GEOMETRY_HPP
