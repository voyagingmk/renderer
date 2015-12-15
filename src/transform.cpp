#include "transform.hpp"

namespace renderer {
	template<class T>
	Matrix<T> Matrix::add(Matrix<T>& m) {
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

	template<class T>
	Matrix<T> Matrix::minus(Matrix<T>& m) {
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
};