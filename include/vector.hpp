#ifndef RENDERER_VECTOR_HPP
#define RENDERER_VECTOR_HPP

#include "base.hpp"


namespace renderer {
	class Vector {
	public:
		float x, y, z;
		static PtrVector Zero;
	public:
		Vector();
		Vector(const float x, const float y, const float z);
		Vector(const Vector &);
		Vector(Vector &&);
		Vector& operator = (const Vector&);
		Vector& operator = (Vector&&);
		~Vector();
		Vector operator + (const Vector&);
		Vector operator - (const Vector&);
		Vector operator * (float f);
		Vector operator / (float f);
		Vector operator - ();
		bool operator == (const Vector& v);
		float dot(const Vector&);
		Vector cross(const Vector&);
		inline float length() { return sqrt(x * x + y * y + z * z); };
		inline float squareLength() { return x * x + y * y + z * z; };
		Vector normalize();
		Vector& swap(Vector&);
		void debug() {
			debug("");
		}
		void debug(std::string msg) {
			//printf("%s [x, y, z = %f, %f, %f]\n", msg.c_str(), m_x, m_y, m_z);
		}


	};
}

#endif // RENDERER_VECTOR_HPP
