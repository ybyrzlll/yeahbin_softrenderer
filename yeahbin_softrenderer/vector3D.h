#pragma once
#include "mMath.h"

// 计算插值：t 为 [0, 1] 之间的数值
static float interp(float x1, float x2, float t) { return x1 + (x2 - x1) * t; }

template<typename T>
struct Vector2 {
	T u, v;
	Vector2() :u(0), v(0) {};
	Vector2(T a, T b) :u(a), v(b) {};

	Vector2 operator-(const Vector2 &rhs) const
	{
		return Vector2(u - rhs.u, v - rhs.v);
	}
	Vector2 operator+(const Vector2 &rhs) const
	{
		return Vector2(u + rhs.u, v + rhs.v);
	}
	Vector2 operator*(const T rhs) const
	{
		return Vector2(u * rhs, v * rhs);
	}
};
typedef Vector2<float> Vector2f;

template<typename T>
struct Vector3 {
	union {
		//Anonymous struct and array union hack
		T data[4];
		struct {
			T x;
			T y;
			T z;
			T w;
		};
	};
	Vector3() :x(0), y(0), z(0), w(1) {};
	Vector3(T val) : x(val), y(val), z(val), w(1) {};
	Vector3(T a, T b, T c) :x(a), y(b), z(c), w(1) {};
	Vector3(T a, T b, T c, T d) :x(a), y(b), z(c), w(d) {};

	//Vector-vector operations
	Vector3 operator-(const Vector3 &rhs) const
	{
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}
	Vector3 operator+(const Vector3 &rhs) const
	{
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}
	Vector3 operator*(const T &rhs) const
	{
		return Vector3(x * rhs, y * rhs, z * rhs, w * rhs);
	}

	void operator+=(const Vector3 &rhs) const
	{
		x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w;
	}

	T  dotProduct(const Vector3 &rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	Vector3 operator*(const Vector3 &rhs) const
	{
		return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	T length() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vector3 &normalized() {
		T len = length();
		T factor = 1 / len;
		x *= factor;
		y *= factor;
		z *= factor;
		return *this;
	}

};

typedef Vector3<float> Vector3f;
typedef Vector3<int> Vector3i;

// | v |
static float vector_length(const Vector3f *v) {
	float sq = v->x * v->x + v->y * v->y + v->z * v->z;
	return (float)sqrt(sq);
}

// z = x + y
static void vector_add(Vector3f *z, Vector3f *x, const Vector3f *y) {
	z->x = x->x + y->x;
	z->y = x->y + y->y;
	z->z = x->z + y->z;
	z->w = 1.0;
}

// z = x - y
static void vector_sub(Vector3f *z, const Vector3f *x, const Vector3f *y) {
	z->x = x->x - y->x;
	z->y = x->y - y->y;
	z->z = x->z - y->z;
	z->w = 1.0;
}

// 矢量点乘
static float vector_dotproduct(const Vector3f *x, const Vector3f *y) {
	return x->x * y->x + x->y * y->y + x->z * y->z;
}

// 矢量叉乘
static void vector_crossproduct(Vector3f *z, const Vector3f *x, const Vector3f *y) {
	float m1, m2, m3;
	m1 = x->y * y->z - x->z * y->y;
	m2 = x->z * y->x - x->x * y->z;
	m3 = x->x * y->y - x->y * y->x;
	z->x = m1;
	z->y = m2;
	z->z = m3;
	z->w = 1.0f;
}

// 矢量插值，t取值 [0, 1]
static void vector_interp(Vector3f *z, const Vector3f *x1, const Vector3f *x2, float t) {
	z->x = interp(x1->x, x2->x, t);
	z->y = interp(x1->y, x2->y, t);
	z->z = interp(x1->z, x2->z, t);
	z->w = 1.0f;
}

// 矢量归一化
static void vector_normalize(Vector3f *v) {
	float length = vector_length(v);
	if (length != 0.0f) {
		float inv = 1.0f / length;
		v->x *= inv;
		v->y *= inv;
		v->z *= inv;
	}
}

// 矢量缩放
static void vector_zoom(Vector3f *v, const float s) {
	v->x *= s;
	v->y *= s;
	v->z *= s;
}