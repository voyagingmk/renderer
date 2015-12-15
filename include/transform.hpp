#ifndef RENDERER_TRANSFORM_HPP
#define RENDERER_TRANSFORM_HPP

#include "base.hpp"

namespace renderer {
	template<class T>
	class Matrix final {
	public:
		T * data;
		typedef typename T::ValueType V;
	public:
		Matrix() {
			data = GetPool<T>()->newElement(T());
		}
		Matrix(std::initializer_list<V> values) {
			data = GetPool<T>()->newElement(T(std::forward<std::initializer_list<V>>(values)));
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
		//access only
		inline const V& operator [] (const int i) { return static_cast<V>(*((V*)data + i)); }
		//API
		Matrix<T> add(Matrix<T>& m) {
			Matrix<T> result;
			int row = T::row, col = T::col;
			int idx;
			V* data = result.data->data;
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					idx = i * col + j;
					data[idx] = (*this)[idx] + m[idx];
				}
			}
			return result;
		}
		Matrix<T> minus(Matrix<T>& m) {
			Matrix<T> result;
			int row = T::row, col = T::col;
			int idx;
			V* data = result.data->data;
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					idx = i * col + j;
					data[idx] = (*this)[idx] - m[idx];
				}
			}
			return result;
		}
		//debug
		void debug() {
			int row = T::row, col = T::col;
			int idx;
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					idx = i * col + j;
					printf("%.1f,", (*this)[idx]);
				}
			}
			printf("\n");
		}
	};

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

	typedef MxN<float, 4, 4> Matrix4x4Value;
	typedef Matrix<Matrix4x4Value> Matrix4x4;
}

#endif