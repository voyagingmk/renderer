#ifndef RENDERER_MATRIX_HPP
#define RENDERER_MATRIX_HPP

#include "base.hpp"
#include "com/geometry.hpp"

namespace renderer {

	template<class V, const int M, const int N>
	class MxN {
	public:
		typedef V ValueType;
		static const int row = M;
		static const int col = N;
		V data[M * N];
	public:
		MxN() {
			memset(data, 0, row * col * sizeof(V));
		}
		MxN(std::initializer_list<V> values) {
			int idx = 0;
			for (auto& value : values) {
				data[idx++] = value;
			}
		}
		const V& operator [] (const int i) { return data[i]; }
	};


	template<class T>
	class Matrix final {
	public:
		T * data;
		typedef typename T::ValueType V;
		typedef MxN<V, T::col, T::row> TransT;
	public:
		Matrix() {
			data = GetPool<T>()->newElement(T());
		}
		
		Matrix(std::initializer_list<V> values) {
			data = GetPool<T>()->newElement(T(std::forward<std::initializer_list<V>>(values)));
		}

		Matrix(const Matrix<T>& m) {
			data = GetPool<T>()->newElement(T()); 
			*this = m;
		}

		Matrix(Matrix<T>&& m) {
			data = m.data;
			m.data = nullptr;
		}	
		virtual ~Matrix() {
			if (!data)
				return;
			auto pool = GetPool<T>();
			pool->deleteElement(data);
		}
        
        bool validate() const {
            int rowNum = row(), colNum = col();
            for (int i = 0; i < rowNum; i++) {
                for (int j = 0; j < colNum; j++) {
                    if(std::isnan((*this)[i * colNum + j])) {
                        return false;
                    }
                }
            }
            return true;
        }

		inline int row() const {
			return T::row;
		}
		
		inline int col() const {
			return T::col;
		}
		
		inline bool isSquare() const {
			return row() == col();
		}
		
		inline bool isSameOrder(const Matrix<T>& m) const {
			return row() == m.row() && col() == m.col();
		}
		
		//access only
		inline V& operator [] (const int i) const {
            V* const p = (V*)data + i;
			return *p;
		}
	
		inline const V at(int r, int c) const {
			return static_cast<V>(*((V*)data + r * col() + c));
		}
        inline void set(int r, int c, V val) {
            *((V*)data + r * col() + c) = val;
        }

		Matrix<T>& operator=(const Matrix<T>& m) {
			memcpy(data, m.data, row() * col() * sizeof(V));
			return *this;
		}
        
		Matrix<T>& operator=(Matrix<T>&& m) {
			memcpy(data, m.data, row() * col() * sizeof(V));
			return *this;
		}
        
		Matrix<T> operator+(const Matrix<T>& m) const {
			return add(m);
		}

		Matrix<T> operator-(const Matrix<T>& m) const {
			return minus(m);
		}
		template<int M, int N>
		Matrix<T> operator*(const Matrix<MxN<V, M, N>>& m) const {
			return multiply(m);
		}

		Matrix<T> operator*(const V v) const {
			return multiply(v);
		}

		Matrix<T> operator*(const Matrix<T>& m) const {
			return multiply(m);
		}

		Matrix<T> operator/(const V v) const {
			return divide(v);
		}

		bool operator==(const Matrix<T>& m) const {
			if (!isSameOrder(m))
				return false;
			int rowNum = row(), colNum = col();
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					if (!almost_equal(at(i,j), m.at(i,j)))
						return false;
				}
			}
			return true;
		}

		bool operator!=(const Matrix<T>& m) const {
			return !(operator==(m));
		}

		Matrix<T> clone() const {
			Matrix<T> result = *this;
			return result;
		}

		Matrix<T> transpose() const {
			Matrix<T> result;
			int rowNum = row(), colNum = col();
			V* dataOfResult = result.data->data;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					dataOfResult[j * rowNum + i] = (*this)[i * colNum + j];
				}
			}
			return result;
		}
		
		Matrix<T> add(const Matrix<T>& m) const {
			Matrix<T> result;
			int rowNum = row(), colNum = col();
			int idx;
			V* dataOfResult = result.data->data;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					idx = i * colNum + j;
					dataOfResult[idx] = (*this)[idx] + m[idx];
				}
			}
			return result;
		}
		
		Matrix<T> minus(const Matrix<T>& m) const {
			Matrix<T> result;
			int rowNum = row(), colNum = col();
			int idx;
			V* dataOfResult = result.data->data;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					idx = i * colNum + j;
					dataOfResult[idx] = (*this)[idx] - m[idx];
				}
			}
			return result;
		}
		
		template<int M, int N>
		Matrix<T> multiply(const Matrix<MxN<V, M, N>>& m) const {
			Matrix<MxN<V, T::row, N>> result;
			for (int i = 0; i < m.row(); i++) {
				for (int j = 0; j < m.col(); j++) {
					V total = 0;
					for (int k = 0; k < col(); k++) {
						total += (*this)[i * col() + k] * m[k * m.col() + j];
					}
					result[i * m.col() + j] = total;
				}
			}
			return result;
		}
		
		Matrix<T> multiply(const V v) const {
			Matrix<T> result;
			int rowNum = row(), colNum = col();
			int idx;
			V* dataOfResult = result.data->data;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					idx = i * colNum + j;
					dataOfResult[idx] = (*this)[idx] *v;
				}
			}
			return result;
		}
		
		Matrix<T> divide(const V v) const {
			return multiply(V(1)/v);
		}

		Matrix<T> power(const int pow) const {
			Matrix<T> result = clone();
			for (int i = 0; i < pow - 1; i++) {
				result = result.multiply(*this);
			}
			return result;
		}

		Matrix<T> inverseAsTriangular() {
			Matrix<T> result;
			Matrix<T> E = newIdentity();
			V* dataOfResult = result.data->data;
			Matrix<T> self = clone();
			int rowNum = row(), colNum = col();
			int idx;
			bool isUpper = false;

			if (std::abs(at(rowNum - 1, 0)) <= std::numeric_limits<V>::epsilon()) {
				self = transpose();
				isUpper = true;
			}

			for (int c = 0; c < colNum; c++) {
				for (int r = 0; r < rowNum; r++) {
					idx = r * colNum + c;
					//printf("idx = %d\n", idx);
					V tmp = 0;
					for (int k = 0; k < r; k++) {
						//printf("r * colNum + k = %d\n", r * colNum + k);
						//printf("tmp += %f*%f\n", (*this)[r * colNum + k], dataOfResult[r * colNum + k]);
						tmp += self[r * colNum + k] * dataOfResult[k * colNum + c];
					}
					//printf("tmp = %f\n", tmp);
					//printf("(E[idx] - tmp)/ (*this)[r * colNum + r] = (%f - %f)/%f\n", E[idx],tmp, (*this)[r * colNum + r]);
                    dataOfResult[idx] = (E[idx] - tmp)/ self[r * colNum + r];
					//printf("=> %f\n\n", dataOfResult[idx]);
				}
			}
			if (isUpper)
				return result.transpose();
			return result;
		}

		Matrix<T> inverse() const {
            Matrix<T> tmp;
            Matrix<T> inv;
            std::vector<V> P;
            int ret = LUPDecompose(tmp, P);
            if (ret == 0) {
                std::cout << "[Matrix] inverse failed" << std::endl;
                return inv;
            }
            LUPInvert(tmp, inv, P);
            return inv;
        }
        /*
        Matrix<T> inverse() const {
			Matrix<T> L, U, P;
			LUP(&L, &U, &P);
			Matrix<T> invL = L.inverseAsTriangular();
			Matrix<T> invU = U.inverseAsTriangular();
            Matrix<T> result = invU * invL * P;
			return result;
		}*/

		Matrix<T> pivotize() const {
			if (!isSquare()) {
				return Matrix<T>();
			}
			int colNum = col();
			auto E = newIdentity();
			for (int row = 0; row < colNum; row++) {
				int row_with_max_elem = row;
				int max_elem = (*this)[row * colNum + row];
				for (int r = row_with_max_elem; r < colNum; r++) {
					if (max_elem < (*this)[r * colNum + row]) {
						max_elem = (*this)[r * colNum + row];
						row_with_max_elem = r;
					}
				}
				if (row != row_with_max_elem) {
					for (int col = 0; col < colNum; col++) {
						int tmp = E[row * colNum + col];
						E[row * colNum + col] = E[row_with_max_elem * colNum + col];
						E[row_with_max_elem * colNum + col] = tmp;
					}
				}
			}
			return E;
		}
        
        // https://en.wikipedia.org/wiki/LU_decomposition#LU_factorization_with_Partial_Pivoting
        int LUPDecompose(Matrix<T>& tmp, std::vector<V>& P) const {
            double Tol = 0.001f;
            int i, j, k, imax;
            int N = row();
            double maxA, absA;
            P.resize(N + 1);
            tmp = *this;
            V* A = (V*)(tmp.data);
            
            for (i = 0; i <= N; i++)
                P[i] = i; //Unit permutation matrix, P[N] initialized with N
            
            for (i = 0; i < N; i++) {
                maxA = 0.0;
                imax = i;
                
                for (k = i; k < N; k++)
                    if ((absA = fabs(A[k * N + i])) > maxA) {
                        maxA = absA;
                        imax = k;
                    }
                
                if (maxA < Tol) return 0; //failure, matrix is degenerate
                
                if (imax != i) {
                    //pivoting P
                    j = P[i];
                    P[i] = P[imax];
                    P[imax] = j;
                    
                    //pivoting rows of A
                    for(int u = 0; u < N; u++)  {
                        V v = A[i * N + u];
                        A[i * N + u] = A[imax * N + u];
                        A[imax * N + u] = v;
                    }
                    
                    //counting pivots starting from N (for determinant)
                    P[N]++;
                }
                
                for (j = i + 1; j < N; j++) {
                    A[j * N + i] /= A[i * N + i];
                    
                    for (k = i + 1; k < N; k++)
                        A[j * N + k] -= A[j * N + i] * A[i * N + k];
                }
            }
            
            return 1;  //decomposition done 
        }
        void LUPInvert(Matrix<T>& tmp, Matrix<T>& invA, std::vector<V>& P) const {
            int N = row();
            V* A = (V*)(tmp.data);
            V* IA = (V*)(invA.data);
            
            for (int j = 0; j < N; j++) {
                for (int i = 0; i < N; i++) {
                    if (P[i] == j)
                        IA[i * N + j] = 1.0;
                    else
                        IA[i * N + j] = 0.0;
                    
                    for (int k = 0; k < i; k++)
                        IA[i * N + j] -= A[i * N + k] * IA[k * N + j];
                }
                
                for (int i = N - 1; i >= 0; i--) {
                    for (int k = i + 1; k < N; k++)
                        IA[i * N + j] -= A[i * N + k] * IA[k * N + j];
                    
                    IA[i * N + j] /= A[i * N + i];
                }
            }
        }


        /*
		void LUP( Matrix<T>* retL, Matrix<T>* retU, Matrix<T>* retP) const { //LUP Decomposition
			if (!isSquare()) {
				return;
			}
			int n = col();
			Matrix<T> L;
			Matrix<T> U;
			Matrix<T> P = pivotize();
            printf("P:\n");
            P.debug();
            printf("this:\n");
            this->debug();
            Matrix<T> PA = P.multiply(*this);
            printf("PA:\n");
            PA.debug();

			for (int j = 0; j < n; j++) {
				L[j * n + j] = 1;
				for (int i = 0; i < j + 1; i++) {
					V s1 = 0;
					for (int k = 0; k < j; k++)
						s1 += U[k * n + j] * L[i * n + k];
					U[i * n + j] = PA[i * n + j] - s1;
                }
				for (int i = j; i < n; i++) {
					V s2 = 0;
					for (int k = 0; k < j; k++)
						s2 += U[k * n + j] * L[i * n + k];
                    L[i * n + j] = (PA[i * n + j] - s2) / U[j * n + j];
                };
			}
            printf("L:\n");
            L.debug();
            printf("U:\n");
            U.debug();
			*retP = P;
			*retL = L;
			*retU = U;
		}
        */
		V diagonalMul() {
			V total = 1;
			for (int r = 0; r <T::row; r++) {
				total *= (*this)[r * (T::row + 1)];
			}
			return total;
		}

		int pivotSwapNum() {
			auto m = clone();
			int n = col();
			int num = 0;
			for (int r = 0; r < n;) {
				if (m[r * (n + 1)] != 0) {
					r++;
					continue;
				}
				else {
					while (m[r * (n + 1)] == 0) {
						int c1, c2;
						for (int c = 0; c < n; c++) {
							if (m[r * n + c] > 0) {
								m[r * n + c] = 0;
								c1 = c;
								break;
							}
						}
						for (int c = 0; c < n; c++) {
							if (m[c1 * n + c] > 0) {
								m[c1 * n + c] = 0;
								c2 = c;
								break;
							}
						}
						//exchage c1 and c2 column (just the two number 1)
						m[c1 * n + c1] = 1;
						m[r * n + c2] = 1;
						num++;
					}
				}
			}
			return num;
		}
        /*
		V det() { //determinant
			if (!isSquare()) {
				return 0;
			}
			Matrix<T> L, U, P;
			LUP(&L, &U, &P);
			Matrix<T> Pt = P.transpose();
			V detPt = Pt.pivotSwapNum() % 2 == 0 ? 1 : -1;
			V detL = L.diagonalMul();
			V detU = U.diagonalMul();
			V detA = detPt * detL * detU;
			return detA;
		}*/

		bool IsIdentity() const {
			if (!isSquare()) {
                return false;
			}
			for (int r = 0; r < T::row; r++)
			{
				for (int c = 0; c < T::col; c++)
				{
                    if (r == c && !AlmostEqual(at(r, c), V(1.0))) {
						return false;
                    }
                    if (r != c && !AlmostEqual(at(r, c), V(0.0))) {
						return false;
                    }
				}
			};
			return true;
		}

		static Matrix<T> newIdentity() {
			Matrix<T> result;
			if (T::row != T::col)
				return result;
			for (int r = 0; r < T::row; r++)
			{
				result[r * T::row + r] = V(1);
			};
			return result;
		}
 
		//debug
		void debug() const {
			int rowNum = row(), colNum = col();
			int idx;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					idx = i * colNum + j;
					printf("%.3f\t", (*this)[idx]);
				}
				printf("\n");
			}
			printf("\n");
		}
	};


	typedef MxN<float, 4, 4> Matrix4x4Value;
	typedef Matrix<Matrix4x4Value> Matrix4x4;
	
	typedef MxN<float, 3, 3> Matrix3x3Value;
	typedef Matrix<Matrix3x3Value> Matrix3x3;

	typedef MxN<double, 4, 4> Matrix4x4ValueD;
	typedef Matrix<Matrix4x4ValueD> Matrix4x4D;

	typedef MxN<double, 3, 3> Matrix3x3ValueD;
	typedef Matrix<Matrix3x3ValueD> Matrix3x3D;
    
    
    template<class T>
    static T Translate(const Vector3dF &v) {
        return T{
            1, 0, 0, v.x,
            0, 1, 0, v.y,
            0, 0, 1, v.z,
            0, 0, 0, 1 };
    }
    
    template<class T>
    static T TranslateInv(const Vector3dF &v) {
        return T{
            1, 0, 0, -v.x,
            0, 1, 0, -v.y,
            0, 0, 1, -v.z,
            0, 0, 0, 1 };
    }
    
    template<class T>
    static T Scale(const Vector3dF &v) {
        return T{
            v.x, 0, 0, 0,
            0, v.y, 0, 0,
            0, 0, v.z, 0,
            0, 0, 0, 1.0 };
    }
    
    template<class T>
    static T ScaleInv(const Vector3dF &v) {
        return T{
            1.0f / v.x, 0.f, 0.f, 0.f,
            0.f, 1.0f / v.y, 0.f, 0.f,
            0.f, 0.f, 1.0f / v.z, 0.f,
            0.f, 0.f, 0, 1.0f };
    }
    
    template<class T>
    static T RotateX(float angle) {
        float sin_theta = sinf(Radians(angle));
        float cos_theta = cosf(Radians(angle));
        return T{
            1, 0, 0, 0,
            0, cos_theta, sin_theta, 0,
            0, -sin_theta, cos_theta, 0,
            0, 0, 0, 1 };
    }
    
    template<class T>
    static T RotateY(float angle) {
        float sin_theta = sinf(Radians(angle));
        float cos_theta = cosf(Radians(angle));
        return T{
            cos_theta, 0, -sin_theta, 0,
            0, 1, 0, 0,
            sin_theta, 0, cos_theta, 0,
            0, 0, 0, 1 };
    }
    
    template<class T>
    static T RotateZ(float angle) {
        float sin_theta = sinf(Radians(angle));
        float cos_theta = cosf(Radians(angle));
        return T{
            cos_theta, sin_theta, 0, 0,
            -sin_theta, cos_theta, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 };
    }
    
    template<class T>
    static T Rotate(float angle, const Vector3dF &axis) {
        Vector3dF a = axis.Normalize();
        float s = sinf(Radians(angle));
        float c = cosf(Radians(angle));
        return T{
            a.x * a.x * (1.f - c) + c,
            a.x * a.y * (1.f - c) - a.z * s,
            a.x * a.z * (1.f - c) - a.y * s,
            0,
            
            a.x * a.y * (1.f - c) - a.z * s,
            a.y * a.y * (1.f - c) + c,
            a.y * a.z * (1.f - c) + a.x * s,
            0,
            
            a.x * a.z * (1.f - c) + a.y * s,
            a.y * a.z * (1.f - c) - a.x * s,
            a.z * a.z * (1.f - c) + c,
            0,
            
            0,
            0,
            0,
            1 };
    }
    
    static Matrix4x4 Perspective(float fovy, float aspect, float n, float f)
    {
        float q = 1.0f / tan(Radians(0.5f * fovy));
        float B = (n + f) / (n - f);
        float C = (2.0f * n * f) / (n - f);
        
        return Matrix4x4{
            q / aspect, 0.0f,   0.0f,   0.0f,
            0.0f,       q,      0.0f,   0.0f,
            0.0f,       0.0f,   B,      C,
            0.0f,       0.0f,   -1.0f,  0.0f
        };
    }
    
    static Matrix4x4 LookAt(const Vector3dF &eye, const Vector3dF &target, const Vector3dF &up) {
        Vector3dF focal = (target - eye).Normalize();
        Vector3dF r = (focal.Cross(up)).Normalize();
        Vector3dF u = r.Cross(focal);
        Vector3dF f = -focal;
        return Matrix4x4{
            r.x, r.y, r.z, -r.Dot(eye),
            u.x, u.y, u.z, -u.Dot(eye),
            f.x, f.y, f.z, -f.Dot(eye),
            0,	 0,	  0,    1
        };
    }
    
    static Matrix4x4 Ortho(
     float left, float right,
     float bottom, float top,
     float zNear, float zFar
     )
    {
        const float A = 2.0f / (right - left);
        const float B = 2.0f / (top - bottom);
#define DEPTH_ZERO_TO_ONE 1
#ifdef DEPTH_ZERO_TO_ONE
        const float C =-1.0f / (zFar - zNear);
#else
        const float C =-2.0f / (zFar - zNear);
#endif
        const float D =-(right + left) / (right - left);
        const float E =-(top + bottom) / (top - bottom);
#ifdef DEPTH_ZERO_TO_ONE
        const float F =-zNear / (zFar - zNear);
#else
        const float F =-(zFar + zNear) / (zFar - zNear);
#endif
        Matrix4x4 result{
        A,      0.0f,   0.0f,   D,
        0.0f,   B,      0.0f,   E,
        0.0f,   0.0f,   C,      F,
        0.0f,   0.0f,   0.0f,   1.0f
        };
        return result;
    }

}

#endif
