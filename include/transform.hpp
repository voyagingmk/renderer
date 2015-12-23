#ifndef RENDERER_TRANSFORM_HPP
#define RENDERER_TRANSFORM_HPP

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

		Matrix(Matrix<T>& m) {
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

		inline int row() {
			return T::row;
		}
		
		inline int col() {
			return T::col;
		}
		
		inline bool isSquare() {
			return row() == col();
		}
		
		inline bool isSameOrder(Matrix<T>& m) {
			return row() == m.row() && col() == m.col();
		}
		
		//access only
		inline V& operator [] (const int i) { 
			return static_cast<V>(*((V*)data + i)); 
		}
	
		inline const V at(int r, int c) {
			return static_cast<V>(*((V*)data + r * col() + c));
		}

		Matrix<T>& operator=(Matrix<T>& m) {
			memcpy(data, m.data, row() * col() * sizeof(V));
			return *this;
		}
		Matrix<T>& operator=(Matrix<T>&& m) {
			data = m.data;
			m.data = nullptr;
			return *this;
		}
		Matrix<T> operator+(Matrix<T>& m) {
			return add(m);
		}

		Matrix<T> operator-(Matrix<T>& m) {
			return minus(m);
		}

		Matrix<T> operator*(V v) {
			return multiply(v);
		}

		Matrix<T> operator*(Matrix<T>& m) {
			return multiply(m);
		}

		Matrix<T> operator/(V v) {
			return divide(v);
		}
		//API
		Matrix<T> clone() {
			Matrix<T> result = *this;
			return result;
		}

		Matrix<TransT> transpose() {
			Matrix<TransT> result;
			int rowNum = row(), colNum = col();
			V* dataOfResult = result.data->data;
			for (int i = 0; i < rowNum; i++) {
				for (int j = 0; j < colNum; j++) {
					dataOfResult[j * rowNum + i] = (*this)[i * colNum + j];
				}
			}
			return result;
		}
		
		Matrix<T> add(Matrix<T>& m) {
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
		
		Matrix<T> minus(Matrix<T>& m) {
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
		Matrix<T> multiply(Matrix<MxN<V, M, N>>& m) {
			Matrix<MxN<V, T::row, N>> result;
			int rowNum = row(), colNum = col();
			int idx;
			V* dataOfResult = result.data->data;
			for (int i = 0; i < m.row(); i++) {
				for (int j = 0; j < m.col(); j++) {
					V total = 0;
					for (int k = 0; k < colNum; k++) {
						total += (*this)[i * colNum + k] * m[k * m.col() + j];
					}
					dataOfResult[i * m.col() + j] = total;
				}
			}
			return result;
		}
		
		Matrix<T> multiply(V v) {
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
		
		Matrix<T> divide(V v) {
			return multiply(V(1)/v);
		}

		Matrix<T> power(int pow) {
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

		Matrix<T> pivotize() {
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
		void LUP(Matrix<T>* retP, Matrix<T>* retL, Matrix<T>* retU) { //LUP Decomposition
			if (!isSquare()) {
				return;
			}
			int n = col();
			Matrix<T> L;
			Matrix<T> U;
			Matrix<T>* A = this;
			auto P = A->pivotize();
			auto PA = P.multiply(*A);

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
		static Matrix<T> newIdentity() {
			return Matrix<T>{
				1.f, 0, 0, 0,
				0, 1.f, 0, 0,
				0, 0, 1.f, 0,
				0, 0, 0, 1.f
			};
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
}

#endif