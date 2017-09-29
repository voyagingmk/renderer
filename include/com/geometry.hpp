#ifndef RENDERER_GEOMETRY_HPP
#define RENDERER_GEOMETRY_HPP

#include "base.hpp"


namespace renderer {

	template<typename T>
	class Point2d {
	public:
		T x, y;
	public:
		Point2d() :x(0), y(0) {}
		Point2d(const T x, const T y) :x(x), y(y) {};
		Point2d(const Point2d<T> & p) {
			x = p.x;
			y = p.y;
		}
		Point2d<T>& operator = (const Point2d<T>& p) {
			x = p.x;
			y = p.y;
			return *this;
		}
		Point2d<T> operator + (const Point2d<T>& p) {
			return Point2d<T>(x + p.x, y + p.y);
		}
		Point2d<T> operator - (const Point2d<T>& p) {
			return Point2d<T>(x - p.x, y - p.y);
		}
		Point2d<T> operator * (T f) {
			return Point2d<T>(x * f, y * f);
		}
		Point2d<T> operator / (T f) {
			if (f == 0)
				throw "[Point /] f = 0";
			return Point2d<T>(x / f, y / f);
		}
		Point2d<T>& operator += (const Point2d<T>& p) {
			x += p.x;
			y += p.y;
			return *this;
		}
		Point2d<T>& operator -= (const Point2d<T>& p) {
			x -= p.x;
			y -= p.y;
			return *this;
		}
		Point2d<T>& operator *= (T f) {
			x *= f;
			y *= f;
			return *this;
		}
		Point2d<T>& operator /= (T f) {
			x /= f;
			y /= f;
			return *this;
		}
		Point2d<T> operator - () {
			return Point2d<T>(-x, -y);
		}
		bool operator == (const Point2d<T>& p) {
			return x == p.x && y == p.y;
		}
		T operator[](int i) const {
			return (&x)[i];
		}
		T &operator[](int i) {
			return (&x)[i];
		}
		T Dot(const Point2d<T>& p) {
			return x * p.x + y * p.y;
		}
		inline T Length() {
			return sqrt(x * x + y * y);
		};
		inline T LengthSquare() {
			return x * x + y * y;
		};
		Point2d<T> Normalize() {
			T inv = T(1) / Length();
			return Point2d<T>(x * inv, y * inv);
		}
		Point2d& Swap(Point2d& p) {
			std::swap(*this, p);
			return *this;
		}
	};

	template<typename T>
	class Point3d{
	public:
		T x, y, z;
	public:
		inline Point3d():x(0), y(0), z(0) {}

		Point3d(const T x, const T y, const T z):x(x), y(y), z(z) {};
		
		Point3d(const Point3d<T>&& p) :
			x(p.x),
			y(p.y),
			z(p.z) {
		}

		Point3d(const Point3d<T> & p):
			x(p.x),
			y(p.y),
			z(p.z) {
		}

		~Point3d() {

		}

		inline Point3d<T>& operator = (const Point3d<T>& p) {
			x = p.x;
			y = p.y;
			z = p.z;
			return *this;
		}

		inline Point3d<T>& operator = (const Point3d<T>&& p) {
			x = p.x;
			y = p.y;
			z = p.z;
			return *this;
		}

		inline Point3d<T> operator + (const Point3d<T>& p) const {
			return Point3d<T>(x + p.x, y + p.y, z + p.z);
		}

		inline Point3d<T> operator - (const Point3d<T>& p) const {
			return Point3d<T>(x - p.x, y - p.y, z - p.z);
		}

		inline Point3d<T> operator * (T f) const {
			return Point3d<T>(x * f, y * f, z * f);
		}

		friend Point3d<T> operator * (T f, Point3d<T> p) {
			return Point3d<T>(p.x * f, p.y * f, p.z * f);
		}

		inline Point3d<T> operator / (T f) const {
			if (f == 0)
				throw "[Point /] f = 0";
			return Point3d<T>(x / f, y / f, z / f);
		}

		inline Point3d<T>& operator += (const Point3d<T>& p) {
			x += p.x;
			y += p.y;
			z += p.z;
			return *this;
		}

		inline Point3d<T>& operator -= (const Point3d<T>& p) {
			x -= p.x;
			y -= p.y;
			z -= p.z;
			return *this;
		}

		inline Point3d<T>& operator *= (const T f) {
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		inline Point3d<T>& operator /= (const T f) {
			x /= f;
			y /= f;
			z /= f;
			return *this;
		}

		inline Point3d<T> operator - () const {
			return Point3d<T>(-x, -y, -z);
		}
        
        inline bool operator != (const Point3d<T>& p) {
            return !AlmostEqual(x, p.x) || !AlmostEqual(y, p.y) || !AlmostEqual(z, p.z);
        }
        
		inline bool operator == (const Point3d<T>& p) const {
			return AlmostEqual(x, p.x) && AlmostEqual(y, p.y) && AlmostEqual(z, p.z);
		}

		inline T operator[](int i) const {
			return (&x)[i];
		}

		inline T &operator[](int i) {
			return (&x)[i];
		}
        
        inline T operator[](Axis i) const {
            return (&x)[static_cast<int>(i)];
        }
        
        inline T &operator[](Axis i) {
            return (&x)[static_cast<int>(i)];
        }
        
        inline bool isEmpty() const {
			return !(x || y || z);
		}

		inline T Dot(const Point3d<T>& p) const {
			return x * p.x + y * p.y + z * p.z;
		}

		Point3d<T> Cross(const Point3d<T> & p) const {
			return Point3d<T>(-z * p.y + y * p.z, z * p.x - x * p.z, -y * p.x + x * p.y);
		}

		inline T Length() const {
			return sqrt(x * x + y * y + z * z); 
		};

		inline T LengthSquare() const {
			return x * x + y * y + z * z; 
		};

		Point3d<T> rotate(Point3d<T> &axis, float angle) const {
			//v' = cos¦È(v - (v¡¤n)n) + sin¦È(n¡Áv) + (v¡¤n)n
			//p = (v¡¤n)n
			//v' = cos¦È(v - p) + sin¦È(n¡Áv) + p
			Point3d<T> p = axis * Dot(axis);
			return ((*this) - p) * cos(angle) + axis.Cross(*this) * sin(angle) + p;
		}

		Point3d<T> Normalize() const {
			T len = Length();
            if (len == 0) {
                return *this;
            }
			T inv = T(1) / len;
			return Point3d<T>(x * inv, y * inv, z * inv);
		}

		Point3d& Swap(Point3d& p) {
			std::swap(*this, p);
			return *this;
		}
        void debug() {
            printf("%f  %f  %f\n", x, y, z);
        }
	};

    typedef Point2d<float> Point2dF;
    typedef Point2d<int> Point2dI;
	typedef Point3d<float> Point3dF;

	using Vector2dF = Point2dF;

	template<typename T>
	class Normal2d :public Point2d<T> {
		using Point2d<T>::Point2d;
	};

	template<typename T>
	class Vector3d :public Point3d<T> {
	public:
		Vector3d():
			Point3d<T>()
		{}

		Vector3d(const T x, const T y, const T z):
			Point3d<T>(x, y, z) 
		{}

		Vector3d(const Vector3d<T>&& p) {
            Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Vector3d(const Vector3d<T>& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Vector3d(const Point3d<T>&& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Vector3d(const Point3d<T>& p)
		{
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}
        
        inline bool operator != (const Vector3d<T>& p) {
            return !AlmostEqual(Point3d<T>::x, p.x) || !AlmostEqual(Point3d<T>::y, p.y) || !AlmostEqual(Point3d<T>::z, p.z);
        }
        
        inline bool operator == (const Vector3d<T>& p) const {
            return AlmostEqual(Point3d<T>::x, p.x) && AlmostEqual(Point3d<T>::y, p.y) && AlmostEqual(Point3d<T>::z, p.z);
        }

		Vector3d<T>& operator = (const Vector3d<T>&& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
			return *this;
		}

		Vector3d<T>& operator = (const Vector3d<T>& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
			return *this;
		}
        
    };
    typedef Vector3d<float> Vector3dF;

	template<typename T>
	class Normal3d :public Point3d<T> {
	public:
		using Point3d<T>::Point3d;
		Normal3d():
			Point3d<T>()
		{
		}
        
        Normal3d(const T x, const T y, const T z) {
            Point3d<T>::x = x;
            Point3d<T>::y = y;
            Point3d<T>::z = z;
        };
        
		Normal3d(const Normal3d<T>&& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Normal3d(const Normal3d<T>& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Normal3d(const Vector3d<T>&& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Normal3d(const Vector3d<T>& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Normal3d(const Point3d<T>&& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}

		Normal3d(const Point3d<T>& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
		}
		Normal3d<T>& operator = (const Normal3d<T>& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
			return *this;
		}		
		Normal3d<T>& operator = (const Normal3d<T>&& p) {
			Point3d<T>::x = p.x;
			Point3d<T>::y = p.y;
			Point3d<T>::z = p.z;
			return *this;
		}
	};

	typedef Normal2d<float> Normal2dF;
	typedef Normal3d<float> Normal3dF;

	namespace Const {
		static Point3dF Zero;
	}

	typedef std::vector<Vector3dF> VectorArray;
	typedef std::vector<Normal3dF> NormalArray;
	typedef std::vector<Vector2dF> UVArray;
	typedef std::vector<uint> UIntArray;


}

#endif // RENDERER_GEOMETRY_HPP
