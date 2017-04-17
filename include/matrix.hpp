#ifndef RENDERER_MATRIX_HPP
#define RENDERER_MATRIX_HPP

#include "base.hpp"

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

		Matrix<T>& operator=(const Matrix<T>& m) {
			memcpy(data, m.data, row() * col() * sizeof(V));
			return *this;
		}
		Matrix<T>& operator=(Matrix<T>&& m) {
			data = m.data;
			m.data = nullptr;
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
			int rowNum = row(), colNum = col();
			int idx;
			for (int i = 0; i < m.row(); i++) {
				for (int j = 0; j < m.col(); j++) {
					V total = 0;
					for (int k = 0; k < colNum; k++) {
						total += (*this)[i * colNum + k] * m[k * m.col() + j];
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
			Matrix<T> L, U, P;
			LUP(&L, &U, &P);
			Matrix<T> invL = L.inverseAsTriangular();
			Matrix<T> invU = U.inverseAsTriangular();
			Matrix<T> result = invU * invL * P;
			return result;
		}

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

		void LUP( Matrix<T>* retL, Matrix<T>* retU, Matrix<T>* retP) const { //LUP Decomposition
			if (!isSquare()) {
				return;
			}
			int n = col();
			Matrix<T> L;
			Matrix<T> U;
			Matrix<T> P = pivotize();
			Matrix<T> PA = P.multiply(*this);

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
				}
			}
			*retP = P;
			*retL = L;
			*retU = U;
		}
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
		}

		bool IsIdentity() const {
			if (!isSquare()) {
				return false;
			}
			for (int r = 0; r < T::row; r++)
			{
				for (int c = 0; c < T::col; c++)
				{
					if (r == c && !almost_equal(at(r, c), 1.f))
						return false;
					if (r != c && !almost_equal(at(r, c), 0.f))
						return false;
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
		void debug() {
			int rowNum = row(), colNum = col();
			int idx;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					idx = i * colNum + j;
					printf("%.1f\t", (*this)[idx]);
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

}

#endif
