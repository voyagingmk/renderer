#ifndef RENDERER_TRANSFORM_HPP
#define RENDERER_TRANSFORM_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "matrix.hpp"
#include "bbox.hpp"

namespace renderer {

	class Transform4x4 {
	public:
		Matrix4x4 m, mInv;
	public:
		Transform4x4() { }

		Transform4x4(const float mat[4][4]) {
			m = Matrix4x4{ 
				mat[0][0], mat[0][1], mat[0][2], mat[0][3],
				mat[1][0], mat[1][1], mat[1][2], mat[1][3],
				mat[2][0], mat[2][1], mat[2][2], mat[2][3],
				mat[3][0], mat[3][1], mat[3][2], mat[3][3] };
			mInv = m.inverse();
		}

		Transform4x4(const Matrix4x4 &mat)
			: m(mat), mInv(mat.inverse()) {
		}

		Transform4x4(const Matrix4x4 &mat, const Matrix4x4 &minv)
			: m(mat), mInv(minv) {
		}

		friend Transform4x4 Inverse(const Transform4x4 &t) {
			return Transform4x4(t.mInv, t.m);
		}

		friend Transform4x4 Transpose(const Transform4x4 &t) {
			return Transform4x4(t.m.transpose(), t.mInv.transpose());
		}

		bool operator==(const Transform4x4 &t) const {
			return t.m == m && t.mInv == mInv;
		}

		bool operator!=(const Transform4x4 &t) const {
			return t.m != m || t.mInv != mInv;
		}

		bool operator<(const Transform4x4 &t2) const {
			for (uint32_t i = 0; i < 4; ++i)
				for (uint32_t j = 0; j < 4; ++j) {
					if (m.at(i, j) < t2.m.at(i, j)) return true;
					if (m.at(i, j) > t2.m.at(i, j)) return false;
				}
			return false;
		}

		bool IsIdentity() const {
			return m.IsIdentity();
		}

		const Matrix4x4 &GetMatrix() const { return m; }

		const Matrix4x4 &GetInverseMatrix() const { return mInv; }

		bool HasScale() const {
			float la2 = (*this)(Vector3dF(1, 0, 0)).LengthSquare();
			float lb2 = (*this)(Vector3dF(0, 1, 0)).LengthSquare();
			float lc2 = (*this)(Vector3dF(0, 0, 1)).LengthSquare();
#define NOT_ONE(x)  !almost_equal(x, 1.f)
			return (NOT_ONE(la2) || NOT_ONE(lb2) || NOT_ONE(lc2));
#undef NOT_ONE
		}
        
		inline Point3dF operator()(const Point3dF &v) const;
        
		inline Vector3dF operator()(const Vector3dF &v) const;
        
		inline Normal3dF operator()(const Normal3dF &v) const;
        
		inline Ray operator()(const Ray &r) const;
        
		inline BBox operator()(const BBox &b) const;
        
        Transform4x4 operator*(const Transform4x4 &t) const{
            return Transform4x4(m * t.m, t.mInv * mInv);
        }
        
        ////////////////////////////////////////////////////
        //  kinds of Transform
        ////////////////////////////////////////////////////
        static Transform4x4 Translate(const Vector3dF &v) {
            return Transform4x4(
                renderer::Translate<Matrix4x4>(v),
                renderer::TranslateInv<Matrix4x4>(v));
        }
        
        static Transform4x4 Scale(const Vector3dF &v) {
            return Transform4x4(
                renderer::Scale<Matrix4x4>(v),
                renderer::ScaleInv<Matrix4x4>(v));
        }
        
        static Transform4x4 RotateX(float angle) {
            auto m = renderer::RotateX<Matrix4x4>(angle);
            return Transform4x4(m, m.transpose());
        }
        
        static Transform4x4 RotateY(float angle) {
            auto m = renderer::RotateY<Matrix4x4>(angle);
            return Transform4x4(m, m.transpose());
        }
        
        static Transform4x4 RotateZ(float angle) {
            auto m = renderer::RotateZ<Matrix4x4>(angle);
            return Transform4x4(m, m.transpose());
        }
        
        static Transform4x4 Rotate(float angle, const Vector3dF &axis) {
            auto m = renderer::Rotate<Matrix4x4>(angle, axis);
            return Transform4x4(m, m.transpose());
        }
    };
    
    inline Point3dF Transform4x4::operator()(const Point3dF &v) const {
		float x = v.x, y = v.y, z = v.z;
		return Point3dF(
			m.at(0, 0) * x + m.at(0, 1) * y + m.at(0, 2) * z + m.at(0, 3),
			m.at(1, 0) * x + m.at(1, 1) * y + m.at(1, 2) * z + m.at(1, 3),
			m.at(2, 0) * x + m.at(2, 1) * y + m.at(2, 2) * z + m.at(2, 3));
	}

	inline Vector3dF Transform4x4::operator()(const Vector3dF &v) const {
		float x = v.x, y = v.y, z = v.z;
		return Vector3dF(
			m.at(0, 0) * x + m.at(0, 1) * y + m.at(0, 2) * z,
			m.at(1, 0) * x + m.at(1, 1) * y + m.at(1, 2) * z,
			m.at(2, 0) * x + m.at(2, 1) * y + m.at(2, 2) * z);
	}

	inline Normal3dF Transform4x4::operator()(const Normal3dF &n) const {
		float x = n.x, y = n.y, z = n.z;
		return Normal3dF(
			mInv.at(0, 0) * x + mInv.at(1, 0) * y + mInv.at(2, 0) * z,
			mInv.at(0, 1) * x + mInv.at(1, 1) * y + mInv.at(2, 1) * z,
			mInv.at(0, 2) * x + mInv.at(1, 2) * y + mInv.at(2, 2) * z);
	}

	inline Ray Transform4x4::operator()(const Ray &r) const {
		Ray ret(r);
		const Transform4x4 &T = *this;
		ret.o = T(ret.o);
		ret.d = T(ret.d);
		return ret;
	}

	inline BBox Transform4x4::operator()(const BBox &b) const {
		const Transform4x4 &T = *this;
		BBox ret(T(Point3dF(b.pMin.x, b.pMin.y, b.pMin.z)));
		ret = ret.Union(T(Point3dF(b.pMax.x, b.pMin.y, b.pMin.z)));
		ret = ret.Union(T(Point3dF(b.pMin.x, b.pMax.y, b.pMin.z)));
		ret = ret.Union(T(Point3dF(b.pMin.x, b.pMin.y, b.pMax.z)));
		ret = ret.Union(T(Point3dF(b.pMin.x, b.pMax.y, b.pMax.z)));
		ret = ret.Union(T(Point3dF(b.pMax.x, b.pMax.y, b.pMin.z)));
		ret = ret.Union(T(Point3dF(b.pMax.x, b.pMin.y, b.pMax.z)));
		ret = ret.Union(T(Point3dF(b.pMax.x, b.pMax.y, b.pMax.z)));
		return ret;
	}

	typedef Transform4x4 Transform;
}

#endif
