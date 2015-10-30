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
		Point& operator = (Point&&);
		~Point();
		Point operator + (const Point&);
		Point operator - (const Point&);
		Point operator * (float f);
		Point operator / (float f);
		Point operator - ();
		bool operator == (const Point& v);
		float Dot(const Point&);
		Point Cross(const Point&);
		inline float Length() { return sqrt(x * x + y * y + z * z); };
		inline float LengthSquare() { return x * x + y * y + z * z; };
		Point Normalize();
		Point& Swap(Point&);
		void Debug() {
			Debug("");
		}
		void Debug(std::string msg) {
			//printf("%s [x, y, z = %f, %f, %f]\n", msg.c_str(), m_x, m_y, m_z);
		}
	};

}

#endif // RENDERER_GEOMETRY_HPP
