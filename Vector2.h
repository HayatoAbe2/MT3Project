#pragma once
struct Vector2 {
	float x;
	float y;
};

// 2次元ベクトル加算
Vector2 Add(const Vector2& v1, const Vector2& v2) {
	Vector2 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

// 2次元ベクトル減算
Vector2 Subtract(const Vector2& v1, const Vector2& v2) {
	Vector2 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}

// 2次元ベクトルのスカラー倍
Vector2 Multiply(float scalar, const Vector2& v) {
	Vector2 result;
	result.x = scalar * v.x;
	result.y = scalar * v.y;
	return result;
}