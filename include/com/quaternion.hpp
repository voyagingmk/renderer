#ifndef RENDERER_QUATERNION_HPP
#define RENDERER_QUATERNION_HPP

#include "base.hpp"
#include "com/geometry.hpp"
#include "com/matrix.hpp"

namespace renderer {
	template<typename T>
	class Quaternion {
    public:
		float s, x, y, z;
	public:
		Quaternion():
			s(0),x(0),y(0),z(0) {}
		Quaternion(const T s, const T x, const T y, const T z):
			s(s), x(x), y(y), z(z) {}
		Quaternion(const Quaternion<T> & q) {
			s = q.s;
			x = q.x;
			y = q.y;
			z = q.z;
		}
		Quaternion<T>& operator = (const Quaternion<T> q) {
			s = q.s;
			x = q.x;
			y = q.y;
			z = q.z;
			return *this;
		}
		Quaternion<T> operator + (const Quaternion<T> q) {
			return Quaternion<T>(s + q.s, x + q.x, y + q.y, z + q.z);
		}
		Quaternion<T> operator - (const Quaternion<T> q) {
			return Quaternion<T>(s - q.s, x - q.x, y - q.y, z - q.z);
		}
		Quaternion<T> operator * (T f) {
			return Quaternion<T>(s * f, x * f, y * f, z * f);
		}
		Quaternion<T> operator * (Quaternion<T> q) const {
			/*
			= (sa + xai + yaj + zak)(sb + xbi + ybj + zbk)
			= (sasb - xaxb - yayb - zazb)
			+ (saxb + sbxa + yazb - ybza)i
			+ (sayb + sbya + zaxb - zbxa)j
			+ (sazb + sbza + xayb - xbya)k
			*/
			return Quaternion<T>(
				s * q.s - x * q.x - y * q.y - z * q.z,
				s * q.x + q.s * x + y * q.z - q.y * z,
				s * q.y + q.s * y + z * q.x - q.z * x,
				s * q.z + q.s * z + x * q.y - q.x * y
 				);
		}
		Quaternion<T> operator / (T f) {
			if (f == 0)
				throw "[Quaternion /] f = 0";
			return Quaternion<T>(s / f, x / f, y / f, z / f);
		}
		Quaternion<T>& operator += (const Quaternion<T> q) {
			s += q.s; 
			x += q.x;
			y += q.y;
			z += q.z;
			return *this;
		}
		Quaternion<T>& operator -= (const Quaternion<T> q) {
			s -= q.s;
			x -= q.x;
			y -= q.y;
			z -= q.z;
			return *this;
		}
		Quaternion<T>& operator *= (T f) {
			s *= f;
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}
		Quaternion<T>& operator /= (T f) {
			s /= f;
			x /= f;
			y /= f;
			z /= f;
			return *this;
		}
		Quaternion<T> operator - () {
			return Quaternion<T>(-s, -x, -y, -z);
		}
		bool operator == (const Quaternion<T> q) {
			return s == q.s && x == q.x && y == q.y && z == q.z;
		}
        Quaternion<T>& operator *= (Quaternion<T>& q) {
            *this = (*this) * q;
            return *this;
        }
        T Norm() const {
			return sqrt(s * s + x * x + y * y + z * z);
		}
		Quaternion<T> Normalize() const {
			T n = Norm();
			assert(n != 0);
			T inv = T(1) / n;
			return Quaternion<T>(s * inv, x * inv, y * inv, z * inv);
		}
		Quaternion<T> Conjugate() const {
			return Quaternion<T>(s, -x, -y, -z);
		}
		Quaternion<T> Inverse() const {
			T n = Norm();
			return Conjugate() / (n * n);
		}
		T Dot(const Quaternion<T>& q) const {
			return s * q.s + x * q.x + y * q.y + z * q.z;
		}
		T CosTheta(const Quaternion<T>& q) const {
			return Dot(q) / (Norm() * q.Norm());
		}
		Quaternion<T> Rotate(const Quaternion<T> rotQ) const {
			const Quaternion<T> rotQInv = rotQ.Conjugate();
			return rotQ * (*this) * rotQInv;
		}
        
		Quaternion<T> Exp() const {
			T normV = sqrt(x * x + y * y + z * z);
			T expS = exp(s);
			T s = expS * cos(normV);
			T f = expS * sin(normV) / normV;
			return Quaternion<T>(s, x * f, y * f, z * f);
		}
		Quaternion<T> Slerp() const {

		}
        
        void debug() {
            printf("%f, %f, %f, %f\n", s, x, y, z);
        }
        
        // x-axis clockwise
        static Quaternion<T> RotateX(T angle) {
            float pitchRadian = T(0.5) * Radians(angle);
            return Quaternion<T>{T(cos(pitchRadian)), T(sin(pitchRadian)), T(0.0), T(0.0)};
        }
        
        // y-axis clockwise
        static Quaternion<T> RotateY(T angle) {
            T yawRadian = T(0.5) * Radians(angle);
            return Quaternion<T>{T(cos(yawRadian)), T(0.0), T(sin(yawRadian)), T(0.0)};
        }
        
        // z-axis clockwise
        static Quaternion<T> RotateZ(T angle) {
            T rollRadian = T(0.5) * Radians(angle);
            return Quaternion<T>{T(cos(rollRadian)), T(0.0), T(0.0), T(sin(rollRadian))};
        }
        
        // *this must be normalized
        Matrix4x4 toMatrix4x4() const  {
            float nQ = x*x + y*y + z*z + s*s;
            float n = 0.0f;
            if (nQ > 0.0f) {
                n = 2.0f / nQ;
            }
            float xy = x * y;
            float yz = y * z;
            float xz = x * z;
            float xs = x * s;
            float ys = y * s;
            float zs = z * s;
            float x2 = x * x;
            float y2 = y * y;
            float z2 = z * z;
            return Matrix4x4{
                1.0f - n*(y2 + z2), n*(xy - zs), n*(xz + ys), 0.0f,
                n*(xy + zs), 1.0f - n*(x2 + z2), n*(yz - xs), 0.0f,
                n*(xz - ys), n*(yz + xs), 1.0f - n*(x2 + y2), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f};
        }
        
        void FromAxis(const T phi, const Axis axis) {
            s = cos(phi/(T(2.0)));
            T sinphi = sin(phi/(T(2.0)));
            Vector3dF a;
            if (axis == Axis::x) {
                a = Vector3dF(1.0f, 0.0f, 0.0f);
            } else if (axis == Axis::y) {
                a = Vector3dF(0.0f, 1.0f, 0.0f);
            } else if (axis == Axis::z) {
                a = Vector3dF(0.0f, 0.0f, 1.0f);
            }
            x = a[0] * sinphi;
            y = a[1] * sinphi;
            z = a[2] * sinphi;
        }

        
        void FromAxis(const T phi, const Vector3dF a) {
            a.Normalize();
            s = cos(phi/(T(2.0)));
            T sinphi = sin(phi/(T(2.0)));
            
            x = a[0] * sinphi;
            y = a[1] * sinphi;
            z = a[2] * sinphi;
        }
   
		// copy from vcglib
        
        void ToEulerAngles(RadianF &rx, RadianF &ry, RadianF &rz) const
        {
#define P(a,b,c,d) (2*((a)*(b)+(c)*(d)))
#define M(a,b,c,d) (2*((a)*(b)-(c)*(d)))
            rx.radian = atan2( P(s,x,y,z) , 1-P(x,x,y,y) );
            ry.radian = asin ( M(s,y,z,x) );
            rz.radian = atan2( P(s,z,x,y) , 1-P(y,y,z,z) );
#undef P
#undef M
        }
        
        void ToEulerAngles(DegreeF &dx, DegreeF &dy, DegreeF &dz) const
        {
            RadianF a, b, c;
            ToEulerAngles(a, b, c);
            dx.degree = a.ToDegree().degree;
            dy.degree = b.ToDegree().degree;
            dz.degree = c.ToDegree().degree;
        }
        
        void FromEulerAngles(RadianF rx, RadianF ry, RadianF rz)
        {
            T cosX = cos(rx.radian / 2.0);
            T cosY = cos(ry.radian / 2.0);
            T cosZ = cos(rz.radian / 2.0);
            T sinX = sin(rx.radian / 2.0);
            T sinY = sin(ry.radian / 2.0);
            T sinZ = sin(rz.radian / 2.0);
           
            const T cosYcosZ = cosY * cosZ;
            const T sinYcosZ = sinY * cosZ;
            const T cosYsinZ = cosY * sinZ;
            const T sinYsinZ = sinY * sinZ;

            s = cosX * cosYcosZ + sinX * sinYsinZ;
            x = sinX * cosYcosZ - cosX * sinYsinZ;
            y = cosX * sinYcosZ + sinX * cosYsinZ;
            z = cosX * cosYsinZ - sinX * sinYcosZ;
        }
        
        void FromEulerAngles(DegreeF dx, DegreeF dy, DegreeF dz)
        {
            FromEulerAngles(dx.ToRadian(), dy.ToRadian(), dz.ToRadian());
        }

        
        void RotationBetweenVectors(Vector3dF start, Vector3dF dest) {
            start = start.Normalize();
            dest = dest.Normalize();
            
            float cosTheta = start.Dot(dest);
            Vector3dF rotationAxis;
            
            if (cosTheta < -1 + 0.001f){
                // special case when vectors in opposite directions:
                // there is no "ideal" rotation axis
                // So guess one; any will do as long as it's perpendicular to start
                rotationAxis = Vector3dF(0.0f, 0.0f, 1.0f).Cross(start);
                if (rotationAxis.LengthSquare() < 0.01 ) // bad luck, they were parallel, try again!
                    rotationAxis = Vector3dF(1.0f, 0.0f, 0.0f).Cross(start);
                
                rotationAxis = rotationAxis.Normalize();
                FromAxis(180.0f, rotationAxis);
            }
            
            rotationAxis = start.Cross(dest);
            
            float d = sqrt( (1+cosTheta)*2 );
            float invd = 1 / d;
            
            s = d * 0.5f;
            x = rotationAxis.x * invd;
            y = rotationAxis.y * invd;
            z = rotationAxis.z * invd;
        }
	};
    
    typedef Quaternion<float> QuaternionF;
}

#endif
