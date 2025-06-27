#pragma once
#include <cmath>

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3& operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
};

// 3次元ベクトル加算
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

// 3次元ベクトル減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

// 3次元ベクトルのスカラー倍
Vector3 Multiply(float scalar, const Vector3& v) {
	Vector3 result;
	result.x = scalar * v.x;
	result.y = scalar * v.y;
	result.z = scalar * v.z;
	return result;
}

// 3次元ベクトルの内積
float Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// 3次元ベクトルの長さ(ノルム)
float Length(const Vector3& v) {
	return sqrtf(Dot(v, v));
}

// 3次元ベクトルの正規化
Vector3 Normalize(const Vector3& v) {
	Vector3 result;
	result.x = v.x / Length(v);
	result.y = v.y / Length(v);
	result.z = v.z / Length(v);
	return result;
}

// 3次元ベクトルのクロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
};

// 線形補間
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return {
		v1.x + (v2.x - v1.x) * t,
		v1.y + (v2.y - v1.y) * t,
		v1.z + (v2.z - v1.z) * t
	};
}

Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return Add(v1, v2);
}

Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	return Subtract(v1, v2);
}

Vector3 operator*(float scalar, const Vector3& v) {
	return Multiply(scalar, v);
}

Vector3 operator*(const Vector3& v, float scalar) {
	return scalar * v;
}

Vector3 operator/(const Vector3& v, float scalar) {
	return Multiply(1.0f / scalar, v);
}

Vector3 operator-(const Vector3& v) {
	return { -v.x, -v.y, -v.z };
}

Vector3 operator+(const Vector3& v) {
	return v;
}