#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "base.hpp"

class Vector {
private:
	float m_x, m_y, m_z;
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
	float dot (const Vector&);
	Vector cross(const Vector&);
	inline float x() const{return m_x;};
	inline float y() const{return m_y;};
	inline float z() const{return m_z;};
	inline float length(){ return sqrt(m_x * m_x + m_y * m_y + m_z * m_z); };
	inline float squareLength(){ return m_x * m_x + m_y * m_y + m_z * m_z; };
	Vector normalize();
	Vector& swap(Vector&);
	void debug(){
        debug("");
    }
    void debug(std::string msg){
        //printf("%s [x, y, z = %f, %f, %f]\n", msg.c_str(), m_x, m_y, m_z);
    }
public:
	static PtrVector Zero;

};

#endif // VECTOR_HPP
