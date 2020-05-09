#pragma once
// https://github.com/samuelpmish/RLUtilities/blob/master/inc/linear_algebra/mat.h
/*
MIT License

Copyright (c) 2019 samuelpmish

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <math.h>
#include <iostream>
#include <initializer_list>

template <int N0, int N1>
class mat {
	float data[N0 * N1];

public:
	mat() {}

	explicit mat(float value) {
		for (int i = 0; i < N0 * N1; i++) {
			data[i] = value;
		}
	}

	mat(std::initializer_list<std::initializer_list<float> > args) {
		int r = 0;
		for (auto row : args) {
			int c = 0;
			for (auto val : row) {
				operator()(r, c) = val;
				c++;
			}
			r++;
		}
	}

	float& operator[](const size_t i) { return data[i]; }
	float operator[](const size_t i) const { return data[i]; }
	float& operator()(const size_t i0, const size_t i1) {
		return data[i0 + N0 * i1];
	}

	float operator()(const size_t i0, const size_t i1) const {
		return data[i0 + N0 * i1];
	}

	mat<N0, N1>& operator=(const mat<N0, N1>& other) {
		for (int i = 0; i < N0 * N1; i++) {
			data[i] = other.data[i];
		}
		return *this;
	}
};

template <int m, int n>
inline mat<n, m> transpose(const mat<m, n>& A) {
	mat<n, m> AT;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			AT(j, i) = A(i, j);
		}
	}
	return AT;
}

template <int m, int n>
inline mat<m, n> operator+(const mat<m, n>& A, const mat<m, n>& B) {
	mat<m, n> C;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			C(i, j) = A(i, j) + B(i, j);
		}
	}
	return C;
}

template <int m, int n>
inline mat<m, n> operator-(const mat<m, n>& A, const mat<m, n>& B) {
	mat<m, n> C;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			C(i, j) = A(i, j) - B(i, j);
		}
	}
	return C;
}



template <int m, int n>
inline mat<m, n> operator*(const mat<m, n>& A, const float other) {
	mat<m, n> B;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			B(i, j) = other * A(i, j);
		}
	}
	return B;
}

template <int m, int n>
inline mat<m, n> operator*(const float other, const mat<m, n>& A) {
	mat<m, n> B;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			B(i, j) = other * A(i, j);
		}
	}
	return B;
}

template <int m, int n>
inline mat<m, n> operator*(const mat<m, n>& A, const mat<m, n>& B) {
	mat<m, n> C;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			C(i, j) = A(i, j) * B(i, j);
		}
	}
	return C;
}

template <int m, int n>
inline mat<m, n> operator/(const mat<m, n>& A, const float denominator) {
	mat<m, n> B;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			B(i, j) = A(i, j) / denominator;
		}
	}
	return B;
}

template <int n>
inline mat<n, n> eye() {
	mat<n, n> I;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			I(i, j) = (i == j);
		}
	}
	return I;
}

template <int m, int n>
inline float fnorm(const mat<m, n>& A) {
	float sum = 0.0;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			sum += A(i, j);
		}
	}
	return sqrt(sum);
}

template <int n>
inline float tr(const mat<n, n>& A) {
	float sum = 0.0;
	for (int i = 0; i < n; i++) {
		sum += A(i, i);
	}
	return sum;
}

inline float det(const mat<2, 2>& A) {
	return A(0, 0) * A(1, 1) - A(0, 1) * A(1, 0);
}

inline float det(const mat<3, 3>& A) {
	return A(0, 0) * A(1, 1) * A(2, 2) + A(0, 1) * A(1, 2) * A(2, 0) +
		A(0, 2) * A(1, 0) * A(2, 1) - A(0, 0) * A(1, 2) * A(2, 1) -
		A(0, 1) * A(1, 0) * A(2, 2) - A(0, 2) * A(1, 1) * A(2, 0);
}

inline float det(const mat<4, 4>& A) {
	return A(0, 3) * A(1, 2) * A(2, 1) * A(3, 0) -
		A(0, 2) * A(1, 3) * A(2, 1) * A(3, 0) -
		A(0, 3) * A(1, 1) * A(2, 2) * A(3, 0) +
		A(0, 1) * A(1, 3) * A(2, 2) * A(3, 0) +
		A(0, 2) * A(1, 1) * A(2, 3) * A(3, 0) -
		A(0, 1) * A(1, 2) * A(2, 3) * A(3, 0) -
		A(0, 3) * A(1, 2) * A(2, 0) * A(3, 1) +
		A(0, 2) * A(1, 3) * A(2, 0) * A(3, 1) +
		A(0, 3) * A(1, 0) * A(2, 2) * A(3, 1) -
		A(0, 0) * A(1, 3) * A(2, 2) * A(3, 1) -
		A(0, 2) * A(1, 0) * A(2, 3) * A(3, 1) +
		A(0, 0) * A(1, 2) * A(2, 3) * A(3, 1) +
		A(0, 3) * A(1, 1) * A(2, 0) * A(3, 2) -
		A(0, 1) * A(1, 3) * A(2, 0) * A(3, 2) -
		A(0, 3) * A(1, 0) * A(2, 1) * A(3, 2) +
		A(0, 0) * A(1, 3) * A(2, 1) * A(3, 2) +
		A(0, 1) * A(1, 0) * A(2, 3) * A(3, 2) -
		A(0, 0) * A(1, 1) * A(2, 3) * A(3, 2) -
		A(0, 2) * A(1, 1) * A(2, 0) * A(3, 3) +
		A(0, 1) * A(1, 2) * A(2, 0) * A(3, 3) +
		A(0, 2) * A(1, 0) * A(2, 1) * A(3, 3) -
		A(0, 0) * A(1, 2) * A(2, 1) * A(3, 3) -
		A(0, 1) * A(1, 0) * A(2, 2) * A(3, 3) +
		A(0, 0) * A(1, 1) * A(2, 2) * A(3, 3);
}

template <int n>
inline float I1(const mat<n, n>& A) {
	return tr(A);
}

template <int n>
inline float I2(const mat<n, n>& A) {
	float trA = tr(A);
	float trAA = 0.0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			trAA += A(i, j) * A(j, i);
		}
	}
	return 0.5 * (trAA - trA * trA);
}

template <int n>
inline float I3(const mat<n, n>& A) {
	return det(A);
}

template <int n>
inline mat<n, n> dev(const mat<n, n>& A) {
	auto A_dev = A;
	float trA = tr(A);
	for (int i = 0; i < n; i++) {
		A_dev(i, i) -= (trA / n);
	}
	return A_dev;
}

inline mat<2, 2> inv(const mat<2, 2>& A) {
	mat<2, 2> invA;

	float inv_detA = 1.0f / det(A);

	invA(0, 0) = A(1, 1) * inv_detA;
	invA(0, 1) = -A(0, 1) * inv_detA;
	invA(1, 0) = -A(1, 0) * inv_detA;
	invA(1, 1) = A(0, 0) * inv_detA;

	return invA;
}

inline mat<3, 3> inv(const mat<3, 3>& A) {
	mat<3, 3> invA;

	float inv_detA = 1.0f / det(A);

	invA(0, 0) = (A(1, 1) * A(2, 2) - A(1, 2) * A(2, 1)) * inv_detA;
	invA(0, 1) = (A(0, 2) * A(2, 1) - A(0, 1) * A(2, 2)) * inv_detA;
	invA(0, 2) = (A(0, 1) * A(1, 2) - A(0, 2) * A(1, 1)) * inv_detA;
	invA(1, 0) = (A(1, 2) * A(2, 0) - A(1, 0) * A(2, 2)) * inv_detA;
	invA(1, 1) = (A(0, 0) * A(2, 2) - A(0, 2) * A(2, 0)) * inv_detA;
	invA(1, 2) = (A(0, 2) * A(1, 0) - A(0, 0) * A(1, 2)) * inv_detA;
	invA(2, 0) = (A(1, 0) * A(2, 1) - A(1, 1) * A(2, 0)) * inv_detA;
	invA(2, 1) = (A(0, 1) * A(2, 0) - A(0, 0) * A(2, 1)) * inv_detA;
	invA(2, 2) = (A(0, 0) * A(1, 1) - A(0, 1) * A(1, 0)) * inv_detA;

	return invA;
}

inline mat<4, 4> inv(const mat<4, 4>& A) {
	mat<4, 4> invA;

	float inv_detA = 1.0f / det(A);

	invA(0, 0) = (A(1, 2) * A(2, 3) * A(3, 1) - A(1, 3) * A(2, 2) * A(3, 1) +
		A(1, 3) * A(2, 1) * A(3, 2) - A(1, 1) * A(2, 3) * A(3, 2) -
		A(1, 2) * A(2, 1) * A(3, 3) + A(1, 1) * A(2, 2) * A(3, 3)) *
		inv_detA;
	invA(0, 1) = (A(0, 3) * A(2, 2) * A(3, 1) - A(0, 2) * A(2, 3) * A(3, 1) -
		A(0, 3) * A(2, 1) * A(3, 2) + A(0, 1) * A(2, 3) * A(3, 2) +
		A(0, 2) * A(2, 1) * A(3, 3) - A(0, 1) * A(2, 2) * A(3, 3)) *
		inv_detA;
	invA(0, 2) = (A(0, 2) * A(1, 3) * A(3, 1) - A(0, 3) * A(1, 2) * A(3, 1) +
		A(0, 3) * A(1, 1) * A(3, 2) - A(0, 1) * A(1, 3) * A(3, 2) -
		A(0, 2) * A(1, 1) * A(3, 3) + A(0, 1) * A(1, 2) * A(3, 3)) *
		inv_detA;
	invA(0, 3) = (A(0, 3) * A(1, 2) * A(2, 1) - A(0, 2) * A(1, 3) * A(2, 1) -
		A(0, 3) * A(1, 1) * A(2, 2) + A(0, 1) * A(1, 3) * A(2, 2) +
		A(0, 2) * A(1, 1) * A(2, 3) - A(0, 1) * A(1, 2) * A(2, 3)) *
		inv_detA;
	invA(1, 0) = (A(1, 3) * A(2, 2) * A(3, 0) - A(1, 2) * A(2, 3) * A(3, 0) -
		A(1, 3) * A(2, 0) * A(3, 2) + A(1, 0) * A(2, 3) * A(3, 2) +
		A(1, 2) * A(2, 0) * A(3, 3) - A(1, 0) * A(2, 2) * A(3, 3)) *
		inv_detA;
	invA(1, 1) = (A(0, 2) * A(2, 3) * A(3, 0) - A(0, 3) * A(2, 2) * A(3, 0) +
		A(0, 3) * A(2, 0) * A(3, 2) - A(0, 0) * A(2, 3) * A(3, 2) -
		A(0, 2) * A(2, 0) * A(3, 3) + A(0, 0) * A(2, 2) * A(3, 3)) *
		inv_detA;
	invA(1, 2) = (A(0, 3) * A(1, 2) * A(3, 0) - A(0, 2) * A(1, 3) * A(3, 0) -
		A(0, 3) * A(1, 0) * A(3, 2) + A(0, 0) * A(1, 3) * A(3, 2) +
		A(0, 2) * A(1, 0) * A(3, 3) - A(0, 0) * A(1, 2) * A(3, 3)) *
		inv_detA;
	invA(1, 3) = (A(0, 2) * A(1, 3) * A(2, 0) - A(0, 3) * A(1, 2) * A(2, 0) +
		A(0, 3) * A(1, 0) * A(2, 2) - A(0, 0) * A(1, 3) * A(2, 2) -
		A(0, 2) * A(1, 0) * A(2, 3) + A(0, 0) * A(1, 2) * A(2, 3)) *
		inv_detA;
	invA(2, 0) = (A(1, 1) * A(2, 3) * A(3, 0) - A(1, 3) * A(2, 1) * A(3, 0) +
		A(1, 3) * A(2, 0) * A(3, 1) - A(1, 0) * A(2, 3) * A(3, 1) -
		A(1, 1) * A(2, 0) * A(3, 3) + A(1, 0) * A(2, 1) * A(3, 3)) *
		inv_detA;
	invA(2, 1) = (A(0, 3) * A(2, 1) * A(3, 0) - A(0, 1) * A(2, 3) * A(3, 0) -
		A(0, 3) * A(2, 0) * A(3, 1) + A(0, 0) * A(2, 3) * A(3, 1) +
		A(0, 1) * A(2, 0) * A(3, 3) - A(0, 0) * A(2, 1) * A(3, 3)) *
		inv_detA;
	invA(2, 2) = (A(0, 1) * A(1, 3) * A(3, 0) - A(0, 3) * A(1, 1) * A(3, 0) +
		A(0, 3) * A(1, 0) * A(3, 1) - A(0, 0) * A(1, 3) * A(3, 1) -
		A(0, 1) * A(1, 0) * A(3, 3) + A(0, 0) * A(1, 1) * A(3, 3)) *
		inv_detA;
	invA(2, 3) = (A(0, 3) * A(1, 1) * A(2, 0) - A(0, 1) * A(1, 3) * A(2, 0) -
		A(0, 3) * A(1, 0) * A(2, 1) + A(0, 0) * A(1, 3) * A(2, 1) +
		A(0, 1) * A(1, 0) * A(2, 3) - A(0, 0) * A(1, 1) * A(2, 3)) *
		inv_detA;
	invA(3, 0) = (A(1, 2) * A(2, 1) * A(3, 0) - A(1, 1) * A(2, 2) * A(3, 0) -
		A(1, 2) * A(2, 0) * A(3, 1) + A(1, 0) * A(2, 2) * A(3, 1) +
		A(1, 1) * A(2, 0) * A(3, 2) - A(1, 0) * A(2, 1) * A(3, 2)) *
		inv_detA;
	invA(3, 1) = (A(0, 1) * A(2, 2) * A(3, 0) - A(0, 2) * A(2, 1) * A(3, 0) +
		A(0, 2) * A(2, 0) * A(3, 1) - A(0, 0) * A(2, 2) * A(3, 1) -
		A(0, 1) * A(2, 0) * A(3, 2) + A(0, 0) * A(2, 1) * A(3, 2)) *
		inv_detA;
	invA(3, 2) = (A(0, 2) * A(1, 1) * A(3, 0) - A(0, 1) * A(1, 2) * A(3, 0) -
		A(0, 2) * A(1, 0) * A(3, 1) + A(0, 0) * A(1, 2) * A(3, 1) +
		A(0, 1) * A(1, 0) * A(3, 2) - A(0, 0) * A(1, 1) * A(3, 2)) *
		inv_detA;
	invA(3, 3) = (A(0, 1) * A(1, 2) * A(2, 0) - A(0, 2) * A(1, 1) * A(2, 0) +
		A(0, 2) * A(1, 0) * A(2, 1) - A(0, 0) * A(1, 2) * A(2, 1) -
		A(0, 1) * A(1, 0) * A(2, 2) + A(0, 0) * A(1, 1) * A(2, 2)) *
		inv_detA;

	return invA;
}

template <int m, int n>
inline mat<m, m> gram(const mat<m, n>& A) {
	mat<m, m> ATA;

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < m; j++) {
			ATA(i, j) = 0.0;
			for (int k = 0; k < n; k++) {
				ATA(i, j) += A(k, i) * A(k, j);
			}
		}
	}

	return ATA;
}

template <int m, int n, int p>
inline mat<m, p> dot(const mat<m, n>& A, const mat<n, p>& B) {
	mat<m, p> C;

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < p; j++) {
			C(i, j) = 0.0;
			for (int k = 0; k < n; k++) {
				C(i, j) += A(i, k) * B(k, j);
			}
		}
	}

	return C;
}

/*
template <int m, int n>
std::ostream& operator<<(std::ostream& os, const mat<m, n>& A) {
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			os << A(i, j);
			if (j == n - 1) os << "\n";
			else          os << ", ";
		}
	}
	return os;
}*/

typedef mat<2, 2> mat2;
typedef mat<3, 3> mat3;
typedef mat<4, 4> mat4;
