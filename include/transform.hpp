#ifndef RENDERER_TRANSFORM_HPP
#define RENDERER_TRANSFORM_HPP

#include "base.hpp"

namespace renderer {
	template<class T>
	class Matrix final {
		T * data;
		typedef typename T::ValueType V;
	public:
		Matrix() {
			data = GetPool<T>()->newElement(T());
		}
		Matrix(std::initializer_list<V> values) {
			data = GetPool<T>()->newElement(T(std::forward<std::initializer_list<V>>(values)));
		}
		virtual ~Matrix() {
			auto pool = GetPool<T>();
			pool->deleteElement(data);
		}
	};

	template<class V, const int M, const int N>
	class MxN {
	public:
		typedef V ValueType;
		static const int m = M;
		static const int n = N;
		V data[M * N];
	public:
		MxN(std::initializer_list<V> values) {
			int idx = 0;
			for (auto& value : values) {
				data[idx++] = value;
			}
		}
	};

	typedef MxN<float, 4, 4> Matrix4x4Value;
	typedef Matrix<Matrix4x4Value> Matrix4x4;
}

#endif