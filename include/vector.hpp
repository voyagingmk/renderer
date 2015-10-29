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
		float Dot(const Vector&);
		Vector Cross(const Vector&);
		inline float Length() { return sqrt(x * x + y * y + z * z); };
		inline float LengthSquare() { return x * x + y * y + z * z; };
		Vector Normalize();
		Vector& Swap(Vector&);
		void Debug() {
			Debug("");
		}
		void Debug(std::string msg) {
			//printf("%s [x, y, z = %f, %f, %f]\n", msg.c_str(), m_x, m_y, m_z);
		}


	};
}

#endif // RENDERER_VECTOR_HPP
