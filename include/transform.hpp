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
		inline const V& operator [] (const int i) { return static_cast<V>(*((V*)data + i)); }
		
		Matrix<T>& operator=(Matrix<T>& m) {
			memcpy(data, m.data, row() * col() * sizeof(V));
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