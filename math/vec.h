#pragma once

// https://github.com/samuelpmish/RLUtilities/blob/master/inc/linear_algebra/vec.h
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

#include "math.h"
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <ostream>
#include <iosfwd>

template <int n>
class vec {
	float data[n];

public:
	vec() {}

	explicit vec(float value) {
		for (int i = 0; i < n; i++) {
			data[i] = value;
		}
	}

	vec(std::initializer_list<float> args) {
		int i = 0;
		for (float arg : args) {
			data[i++] = arg;
			if (i > n) break;
		}
	}

	template <int m>
	vec(const vec<m>& other) {
		for (int i = 0; i < n; i++) {
			data[i] = (i < m) ? other[i] : 0.0f;
		}
	}

	constexpr float& operator[](const size_t i) { return data[i]; }
	constexpr float operator[](const size_t i) const { return data[i]; }
	constexpr float& operator()(const size_t i) { return data[i]; }
	constexpr float operator()(const size_t i) const { return data[i]; }

	// elementwise addition
	vec<n> operator+(const vec<n>& other) const {
		vec<n> v;
		for (int i = 0; i < n; i++) {
			v[i] = data[i] + other[i];
		}
		return v;
	}

	// elementwise multiplication
	vec<n> operator*(const vec<n>& other) const {
		vec<n> v;
		for (int i = 0; i < n; i++) {
			v[i] = data[i] * other[i];
		}
		return v;
	}

	// in-place elementwise addition
	vec<n>& operator+=(const vec<n>& other) {
		for (int i = 0; i < n; i++) {
			data[i] += other[i];
		}
		return *this;
	}

	// unary minus
	vec<n> operator-() const {
		vec<n> v;
		for (int i = 0; i < n; i++) {
			v[i] = -data[i];
		}
		return v;
	}

	// elementwise subtraction
	vec<n> operator-(const vec<n>& other) const {
		vec<n> v;
		for (int i = 0; i < n; i++) {
			v[i] = data[i] - other[i];
		}
		return v;
	}

	// in-place elementwise subtraction
	vec<n>& operator-=(const vec<n>& other) {
		for (int i = 0; i < n; i++) {
			data[i] -= other[i];
		}
		return *this;
	}

	// in-place scalar addition
	vec<n>& operator+=(const float other) {
		for (int i = 0; i < n; i++) {
			data[i] += other;
		}
		return *this;
	}

	// in-place scalar multiplication
	vec<n>& operator*=(const float other) {
		for (int i = 0; i < n; i++) {
			data[i] *= other;
		}
		return *this;
	}

	// in-place scalar division
	vec<n>& operator/=(const float other) {
		for (int i = 0; i < n; i++) {
			data[i] /= other;
		}
		return *this;
	}

	vec<n>& operator=(const vec<n>& other) {
		for (int i = 0; i < n; i++) {
			data[i] = other[i];
		}
		return *this;
	}
};

inline vec<3> cross(const vec<3>& a, const vec<3>& b) {
	return { a(1) * b(2) - a(2) * b(1), a(2) * b(0) - a(0) * b(2),
		a(0) * b(1) - a(1) * b(0) };
}

inline vec<3> cross(const vec<3>& a) { return { -a(1), a(0), 0.0f }; }

inline vec<2> cross(const vec<2>& a) { return { -a(1), a(0) }; }

inline float det(const vec<2>& a, const vec<2>& b) { return a(0) * b(1) - a(1) * b(0); }

template <int n>
inline float norm(const vec<n>& v) {
	return sqrt(dot(v, v));
}

template <int n>
inline vec<n> normalize(const vec<n>& v) {
	float norm_v = norm(v);
	if (norm_v < 1.0e-6) {
		return vec<n>(0.0f);
	}
	else {
		return v / norm_v;
	}
}

template <int n>
inline vec<n> operator*(const vec<n>& v, const float other) {
	vec<n> u;
	for (int i = 0; i < n; i++) {
		u(i) = other * v(i);
	}
	return u;
}

template <int n>
inline vec<n> operator*(const float other, const vec<n>& v) {
	vec<n> u;
	for (int i = 0; i < n; i++) {
		u(i) = other * v(i);
	}
	return u;
}

template <int n>
inline vec<n> operator/(const vec<n>& v, const float other) {
	vec<n> u;
	for (int i = 0; i < n; i++) {
		u(i) = v(i) / other;
	}
	return u;
}

template <int n>
inline vec<n> operator/(const float other, const vec<n>& v) {
	vec<n> u;
	for (int i = 0; i < n; i++) {
		u(i) = other / v(i);
	}
	return u;
}

//inline float atan2(const vec<2>& v) { return atan2(v(1), v(0)); }

template <int n>
inline float dot(const vec<n>& u, const vec<n>& v) {
	float a = 0.0;
	for (int i = 0; i < n; i++) {
		a += u[i] * v[i];
	}
	return a;
}

template <int d>
std::ostream& operator<<(std::ostream& os, const vec<d>& v) {
	for (int i = 0; i < d; i++) {
		os << v[i];
		if (i != d - 1) os << ", ";
	}
	return os;
}

template <int n>
inline vec<n> relu(const vec<n>& v) {
	vec<n> u;
	for (int i = 0; i < n; i++) {
		u(i) = std::max(v(i), 0.f);
	}
	return u;
}

/*
template <int n>
inline vec<n> clamp(const vec<n>& v, const float min_value, const float max_value) {
	vec<n> u;
	for (int i = 0; i < n; i++) {
		u(i) = std::clamp(v(i), min_value, max_value);
	}
	return u;
}*/

typedef vec<2> vec2c;
typedef vec<3> vec3c;
typedef vec<4> vec4c;


#include <bakkesmod/wrappers/WrapperStructs.h>
inline vec3c toVec3(Vector v) {
	vec3c elVec;
	memcpy(&elVec, &v, 0xC);
	return elVec;
}

inline void copyVec3(vec3c* vIn, Vector* v) {
	memcpy(vIn, v, 0xC);
}
