#include <Novice.h>
#include "Matrix4x4.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <imgui.h>
const char kWindowTitle[] = "MT3";

// 球
struct Sphere {
	Vector3 center;
	float radius;
};

//  線分
struct Segment {
	Vector3 origin;
	Vector3 diff;
};

// 直線
struct Line {
	Vector3 origin;
	Vector3 diff;
};

// 半直線
struct Ray {
	Vector3 origin;
	Vector3 diff;
};

// 平面
struct Plane {
	Vector3 normal; // 法線
	float distance; // 距離
};

// 三角形
struct Triangle {
	Vector3 vertices[3];
};

// AABB
struct AABB {
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};

// OBB
struct OBB {
	Vector3 center; // 中心点
	Vector3 orientations[3]; // 各軸の方向ベクトル
	Vector3 size; // 各軸の長さの半分
};

Vector3 Project(const Vector3& v1, const  Vector3& v2);
Vector3 ClosestPoint(const Vector3& point, const Segment& segment);
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
bool CheckCollision(const Sphere& sphere, const Plane& plane);
bool CheckCollision(const Segment& segment, const Plane& plane);
bool CheckCollision(const Line& line, const Plane& plane);
bool CheckCollision(const Ray& ray, const Plane& plane);
bool CheckCollision(const Triangle& triangle, const Segment& segment);
bool CheckCollision(const Triangle& triangle, const Line& line);
bool CheckCollision(const Triangle& triangle, const Ray& ray);
bool CheckCollision(const AABB& aabb1, const AABB& aabb2);
bool CheckCollision(const AABB& aabb, const Sphere& sphere);
bool CheckCollision(const AABB& aabb, const Segment& segment);
Vector3 Perpendicular(const Vector3& vector);

// 表示用の関数
static const int kColumnWidth = 60;
static const int kRowHeight = 20;
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label);
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label);
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4 viewportMatrix);
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);
void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);
void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
	const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// 変数の宣言
	Transform cameraTransform = { {1.0f,1.0f,1.0f},{ 0.26f,0.0f,0.0f },{ 0.0f,1.9f,-6.49f } };
	int kWindowWidth = 1280;
	int kWindowHeight = 720;
	Matrix4x4 viewProjectionMatrix = MakeViewProjectionMatrix(cameraTransform, float(kWindowWidth) / float(kWindowHeight));
	Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

	Vector3 translates[3] = {
		{0.2f,1.0f,0.0f},
		{0.4f,0.0f,0.0f},
		{0.3f,0.0f,0.0f},
	};

	Vector3 rotates[3] = {
		{0.0f,0.0f,-6.8f},
		{0.0f,0.0f,-1.4f},
		{0.0f,0.0f,0.0f},
	};

	Vector3 scales[3] = {
		{1.0f,1.0f,1.0f},
		{1.0f,1.0f,1.0f},
		{1.0f,1.0f,1.0f},
	};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		ImGui::Begin("Window");
		ImGui::DragFloat3("Translates[0]", &translates[0].x, 0.01f);
		ImGui::DragFloat3("Rotates[0]", &rotates[0].x, 0.01f);
		ImGui::DragFloat3("Scales[0]", &scales[0].x, 0.01f);
		ImGui::DragFloat3("Translates[1]", &translates[1].x, 0.01f);
		ImGui::DragFloat3("Rotates[1]", &rotates[1].x, 0.01f);
		ImGui::DragFloat3("Scales[1]", &scales[1].x, 0.01f);
		ImGui::DragFloat3("Translates[2]", &translates[2].x, 0.01f);
		ImGui::DragFloat3("Rotates[2]", &rotates[2].x, 0.01f);
		ImGui::DragFloat3("Scales[2]", &scales[2].x, 0.01f);

		// デバッグ用カメラ操作
		ImGuiIO& io = ImGui::GetIO();
		float moveSpeedDebug = 0.01f;
		float cameraSpeedDebug = 0.001f;
		if (io.KeysDown[ImGuiKey_LeftShift] || io.KeysDown[ImGuiKey_RightShift]) {
			moveSpeedDebug *= 2.0f;
		}

		if (ImGui::IsMouseDown(1) && !ImGui::IsAnyItemActive()) { // 右クリック中
			cameraTransform.rotate.y += io.MouseDelta.x * cameraSpeedDebug;
			cameraTransform.rotate.x += io.MouseDelta.y * cameraSpeedDebug;
		}
		// 移動(初期カメラ向きを基準)
		if (io.KeysDown[ImGuiKey_A]) {
			cameraTransform.translate.x -= moveSpeedDebug; // 左
		}
		if (io.KeysDown[ImGuiKey_D]) {
			cameraTransform.translate.x += moveSpeedDebug; // 右
		}
		if (io.KeysDown[ImGuiKey_Q]) {
			cameraTransform.translate.y -= moveSpeedDebug; // 下
		}
		if (io.KeysDown[ImGuiKey_E]) {
			cameraTransform.translate.y += moveSpeedDebug; // 上
		}
		if (io.KeysDown[ImGuiKey_W]) {
			cameraTransform.translate.z += moveSpeedDebug; // 前
		}
		if (io.KeysDown[ImGuiKey_S]) {
			cameraTransform.translate.z -= moveSpeedDebug; // 後
		}
		ImGui::End();

		Matrix4x4 localMatrix[3];
		localMatrix[0] = MakeAffineMatrix(scales[0], rotates[0], translates[0]);
		localMatrix[1] = MakeAffineMatrix(scales[1], rotates[1], translates[1]);
		localMatrix[2] = MakeAffineMatrix(scales[2], rotates[2], translates[2]);

		Matrix4x4 worldMatrix[3];
		worldMatrix[0] = localMatrix[0];
		worldMatrix[1] = Multiply(localMatrix[1], worldMatrix[0]);
		worldMatrix[2] = Multiply(localMatrix[2], worldMatrix[1]);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// ビューx射影行列の作成
		viewProjectionMatrix = MakeViewProjectionMatrix(cameraTransform, float(kWindowWidth) / float(kWindowHeight));

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// ワールド座標
		Vector3 points[3] = {
		TransformVector({}, worldMatrix[0]),
		TransformVector({}, worldMatrix[1]),
		TransformVector({}, worldMatrix[2])
		};

		// 球描画
		DrawSphere({ points[0],0.03f }, viewProjectionMatrix, viewportMatrix, RED);
		DrawSphere({ points[1],0.03f }, viewProjectionMatrix, viewportMatrix, GREEN);
		DrawSphere({ points[2],0.03f }, viewProjectionMatrix, viewportMatrix, BLUE);

		// スクリーン座標
		Vector3 screenPoints[3] = {
			TransformVector(points[0],Multiply(viewProjectionMatrix,viewportMatrix)),
			TransformVector(points[1],Multiply(viewProjectionMatrix,viewportMatrix)),
			TransformVector(points[2],Multiply(viewProjectionMatrix,viewportMatrix)),
		};

		// 線描画
		Novice::DrawLine(int(screenPoints[0].x), int(screenPoints[0].y), int(screenPoints[1].x), int(screenPoints[1].y), WHITE);
		Novice::DrawLine(int(screenPoints[1].x), int(screenPoints[1].y), int(screenPoints[2].x), int(screenPoints[2].y), WHITE);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}

// 正射影ベクトル
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	return Multiply(Dot(v1, Normalize(v2)), Normalize(v2));
}

// 最近接点
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	return Add(segment.origin, Project(Subtract(point, segment.origin), segment.diff));
}

// 球と平面の衝突判定
bool CheckCollision(const Sphere& sphere, const Plane& plane) {
	// 平面と点の距離
	float k = fabs(Dot(plane.normal, sphere.center) - plane.distance); // 符号なし

	// 半径より距離が小さければ衝突
	return k <= sphere.radius;
}

// 線分と平面の衝突判定
bool CheckCollision(const Segment& segment, const Plane& plane) {
	// 法線と線の内積
	float dot = Dot(plane.normal, segment.diff);
	if (dot == 0.0f) { return false; } // 平行な場合衝突しない

	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;
	if (0 <= t && t <= 1) { // 線分の範囲内なら衝突
		return true;
	}
	return false;
}

// 直線と平面の衝突判定
bool CheckCollision(const Line& line, const Plane& plane) {
	// 法線と線の内積
	float dot = Dot(plane.normal, line.diff);
	if (dot == 0.0f) { return false; } // 平行な場合衝突しない

	// 平行でなければどこかで衝突する
	return true;
}

// 半直線と平面の衝突判定
bool CheckCollision(const Ray& ray, const Plane& plane) {
	// 法線と線の内積
	float dot = Dot(plane.normal, ray.diff);
	if (dot == 0.0f) { return false; } // 平行な場合衝突しない

	float t = (plane.distance - Dot(ray.origin, plane.normal)) / dot;
	if (t >= 0) {
		return true;
	}
	return false;
}

// 三角形と線分の衝突判定
bool CheckCollision(const Triangle& triangle, const Segment& segment) {
	// 三角形から平面を求める
	Plane plane;
	Vector3 v1 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 v2 = Subtract(triangle.vertices[2], triangle.vertices[1]);
	plane.normal = Normalize(Cross(v1, v2)); // 法線
	plane.distance = Dot(triangle.vertices[0], plane.normal); // 平面の距離

	// 衝突点
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / Dot(segment.diff, plane.normal);
	if (0 <= t && t <= 1) { // 線分の範囲内かチェック
		Vector3 p = Add(segment.origin, Multiply(t, segment.diff));

		// 衝突点が三角形の内側かどうかを求める
		Vector3 cross01 = Cross(Subtract(triangle.vertices[1], triangle.vertices[0]), Subtract(p, triangle.vertices[1]));
		Vector3 cross12 = Cross(Subtract(triangle.vertices[2], triangle.vertices[1]), Subtract(p, triangle.vertices[2]));
		Vector3 cross20 = Cross(Subtract(triangle.vertices[0], triangle.vertices[2]), Subtract(p, triangle.vertices[0]));
		// すべての小三角形のクロス積と法線が同じ向きなら衝突
		if (Dot(cross01, plane.normal) >= 0.0f &&
			Dot(cross12, plane.normal) >= 0.0f &&
			Dot(cross20, plane.normal) >= 0.0f) {
			return true;
		}
	}

	return false;
}

// 三角形と直線の衝突判定
bool CheckCollision(const Triangle& triangle, const Line& line) {
	// 三角形から平面を求める
	Plane plane;
	Vector3 v1 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 v2 = Subtract(triangle.vertices[2], triangle.vertices[1]);
	plane.normal = Normalize(Cross(v1, v2)); // 法線
	plane.distance = Dot(triangle.vertices[0], plane.normal); // 平面の距離

	// 衝突点
	float t = (plane.distance - Dot(line.origin, plane.normal)) / Dot(line.diff, plane.normal);
	Vector3 p = Add(line.origin, Multiply(t, line.diff));

	// 衝突点が三角形の内側かどうかを求める
	Vector3 cross01 = Cross(Subtract(triangle.vertices[1], triangle.vertices[0]), Subtract(p, triangle.vertices[1]));
	Vector3 cross12 = Cross(Subtract(triangle.vertices[2], triangle.vertices[1]), Subtract(p, triangle.vertices[2]));
	Vector3 cross20 = Cross(Subtract(triangle.vertices[0], triangle.vertices[2]), Subtract(p, triangle.vertices[0]));
	// すべての小三角形のクロス積と法線が同じ向きなら衝突
	if (Dot(cross01, plane.normal) >= 0.0f &&
		Dot(cross12, plane.normal) >= 0.0f &&
		Dot(cross20, plane.normal) >= 0.0f) {
		return true;
	}
	return false;
}

// 三角形と半直線の衝突判定
bool CheckCollision(const Triangle& triangle, const Ray& ray) {
	// 三角形から平面を求める
	Plane plane;
	Vector3 v1 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 v2 = Subtract(triangle.vertices[2], triangle.vertices[1]);
	plane.normal = Normalize(Cross(v1, v2)); // 法線
	plane.distance = Dot(triangle.vertices[0], plane.normal); // 平面の距離

	// 衝突点
	float t = (plane.distance - Dot(ray.origin, plane.normal)) / Dot(ray.diff, plane.normal);
	if (t >= 0) { // 半直線の範囲内かチェック
		Vector3 p = Add(ray.origin, Multiply(t, ray.diff));

		// 衝突点が三角形の内側かどうかを求める
		Vector3 cross01 = Cross(Subtract(triangle.vertices[1], triangle.vertices[0]), Subtract(p, triangle.vertices[1]));
		Vector3 cross12 = Cross(Subtract(triangle.vertices[2], triangle.vertices[1]), Subtract(p, triangle.vertices[2]));
		Vector3 cross20 = Cross(Subtract(triangle.vertices[0], triangle.vertices[2]), Subtract(p, triangle.vertices[0]));
		// すべての小三角形のクロス積と法線が同じ向きなら衝突
		if (Dot(cross01, plane.normal) >= 0.0f &&
			Dot(cross12, plane.normal) >= 0.0f &&
			Dot(cross20, plane.normal) >= 0.0f) {
			return true;
		}
	}
	return false;
}

// AABB同士の衝突判定
bool CheckCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x軸
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y軸
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {
		return true;
	}
	return false;
}

// AABBと球の衝突判定
bool CheckCollision(const AABB& aabb, const Sphere& sphere) {
	// 最近接点を求める
	Vector3 closestPoint = {
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z),
	};
	// 最近接点と球の中心の距離を求める
	float distance = Length(Subtract(closestPoint, sphere.center));
	// 距離が半径よりも小さければ衝突
	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

bool CheckCollision(const AABB& aabb, const Segment& segment) {
	// 各平面の媒介変数を求める
	Vector3 tMin = {
		(aabb.min.x - segment.origin.x) / segment.diff.x,
		(aabb.min.y - segment.origin.y) / segment.diff.y,
		(aabb.min.z - segment.origin.z) / segment.diff.z
	};
	Vector3 tMax = {
		(aabb.max.x - segment.origin.x) / segment.diff.x,
		(aabb.max.y - segment.origin.y) / segment.diff.y,
		(aabb.max.z - segment.origin.z) / segment.diff.z
	};

	Vector3 tNear = {
		min(tMin.x, tMax.x),
		min(tMin.y, tMax.y),
		min(tMin.z, tMax.z)
	};
	Vector3 tFar = {
		max(tMin.x, tMax.x),
		max(tMin.y, tMax.y),
		max(tMin.z, tMax.z)
	};

	// 衝突(貫通)している点
	float min = max(max(tNear.x, tNear.y), tNear.z);
	float max = min(min(tFar.x, tFar.y), tFar.z);

	// 線分の範囲と衝突しているか
	if (min <= max && (0.0f <= max && min <= 1.0f)) {

		// 衝突
		return true;
	}

	return false;
}

// AABBと直線の衝突判定
bool CheckCollision(const AABB& aabb, const Line& line) {
	// 各平面の媒介変数を求める
	Vector3 tMin = {
		(aabb.min.x - line.origin.x) / line.diff.x,
		(aabb.min.y - line.origin.y) / line.diff.y,
		(aabb.min.z - line.origin.z) / line.diff.z
	};
	Vector3 tMax = {
		(aabb.max.x - line.origin.x) / line.diff.x,
		(aabb.max.y - line.origin.y) / line.diff.y,
		(aabb.max.z - line.origin.z) / line.diff.z
	};

	Vector3 tNear = {
		min(tMin.x, tMax.x),
		min(tMin.y, tMax.y),
		min(tMin.z, tMax.z)
	};
	Vector3 tFar = {
		max(tMin.x, tMax.x),
		max(tMin.y, tMax.y),
		max(tMin.z, tMax.z)
	};

	// 衝突(貫通)している点
	float min = max(max(tNear.x, tNear.y), tNear.z);
	float max = min(min(tFar.x, tFar.y), tFar.z);

	// 直線の範囲と衝突しているか
	if (min <= max) {

		// 衝突
		return true;
	}

	return false;
}

// AABBと半直線の衝突判定
bool CheckCollision(const AABB& aabb, const Ray& ray) {
	// 各平面の媒介変数を求める
	Vector3 tMin = {
		(aabb.min.x - ray.origin.x) / ray.diff.x,
		(aabb.min.y - ray.origin.y) / ray.diff.y,
		(aabb.min.z - ray.origin.z) / ray.diff.z
	};
	Vector3 tMax = {
		(aabb.max.x - ray.origin.x) / ray.diff.x,
		(aabb.max.y - ray.origin.y) / ray.diff.y,
		(aabb.max.z - ray.origin.z) / ray.diff.z
	};

	Vector3 tNear = {
		min(tMin.x, tMax.x),
		min(tMin.y, tMax.y),
		min(tMin.z, tMax.z)
	};
	Vector3 tFar = {
		max(tMin.x, tMax.x),
		max(tMin.y, tMax.y),
		max(tMin.z, tMax.z)
	};

	// 衝突(貫通)している点
	float min = max(max(tNear.x, tNear.y), tNear.z);
	float max = min(min(tFar.x, tFar.y), tFar.z);

	// 半直線の範囲と衝突しているか
	if (min <= max && 0.0f <= max) {

		// 衝突
		return true;
	}

	return false;
}

// 垂直ベクトルを求める
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

// 平面描画
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal)); // 法線と垂直なベクトル
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y, -perpendiculars[0].z }; // の逆ベクトル
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]); // 法線と、垂直ベクトルのクロス積
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y, -perpendiculars[2].z }; // の逆ベクトル
	Vector3 points[4];
	// 頂点を求める
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(2.0f, perpendiculars[index]);
		Vector3 point = Add(center, extend);
		points[index] = TransformVector(TransformVector(point, viewProjectionMatrix), viewportMatrix);
	}
	// 描画
	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
}

// 三角形描画
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 points[3];
	// 頂点を求める
	for (int32_t index = 0; index < 3; ++index) {
		Vector3 point = TransformVector(TransformVector(triangle.vertices[index], viewProjectionMatrix), viewportMatrix);
		points[index] = point;
	}
	// 描画
	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[0].x), int(points[0].y), color);
}

// AABB描画
void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 points[8];
	// 頂点を求める
	points[0] = { aabb.min.x, aabb.min.y, aabb.min.z };
	points[1] = { aabb.max.x, aabb.min.y, aabb.min.z };
	points[2] = { aabb.min.x, aabb.max.y, aabb.min.z };
	points[3] = { aabb.max.x, aabb.max.y, aabb.min.z };
	points[4] = { aabb.min.x, aabb.min.y, aabb.max.z };
	points[5] = { aabb.max.x, aabb.min.y, aabb.max.z };
	points[6] = { aabb.min.x, aabb.max.y, aabb.max.z };
	points[7] = { aabb.max.x, aabb.max.y, aabb.max.z };

	// スクリーン座標に変換
	for (int32_t index = 0; index < 8; ++index) {
		Vector3 point = TransformVector(TransformVector(points[index], viewProjectionMatrix), viewportMatrix);
		points[index] = point;
	}

	// 辺
	static const int edge[12][2] = {
		{0,1},{1,3},{3,2},{2,0}, // 下面
		{4,5},{5,7},{7,6},{6,4}, // 上面
		{0,4},{1,5},{2,6},{3,7}  // 側面
	};

	// 辺を描画
	for (int i = 0; i < 12; ++i) {
		const Vector3& p0 = points[edge[i][0]];
		const Vector3& p1 = points[edge[i][1]];
		Novice::DrawLine(int(p0.x), int(p0.y), int(p1.x), int(p1.y), color);
	}
}

// 2次ベジェ曲線描画
void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	float t = 0.0f;
	const int kSubdivision = 32; // 分割数
	Vector3 p = controlPoint0;
	for (int i = 0; i < kSubdivision; ++i) {
		t = float(i + 1) / float(kSubdivision);
		Vector3 p0p1 = Lerp(controlPoint0, controlPoint1, t); // 制御点0,1を線形補間
		Vector3 p1p2 = Lerp(controlPoint1, controlPoint2, t); // 制御点1,2を線形補間
		Vector3 preP = p;
		p = Lerp(p0p1, p1p2, t); // 二つの点を線形補間

		// スクリーン座標へ変換
		Vector3 screenPoint0 = TransformVector(TransformVector(preP, viewProjectionMatrix), viewportMatrix);
		Vector3 screenPoint1 = TransformVector(TransformVector(p, viewProjectionMatrix), viewportMatrix);

		Novice::DrawLine(int(screenPoint0.x), int(screenPoint0.y), int(screenPoint1.x), int(screenPoint1.y), color);
	}
}

/// <summary>
/// Vector3の各数値を表示
/// </summary>
/// <param name="x">左上座標</param>
/// <param name="y">左上座標</param>
/// <param name="vector">表示するベクトル</param>
/// <param name="label">名前</param>
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

/// <summary>
/// 4x4行列の各要素を表示
/// </summary>
/// <param name="x">左上座標</param>
/// <param name="y">左上座標</param>
/// <param name="matrix">表示する行列</param>
/// <param name="label">名前</param>
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(
				x + column * kColumnWidth, y + row * kRowHeight + kRowHeight, "%6.02f", matrix.m[row][column]);
		}
	}
}

/// <summary>
/// グリッド描画
/// </summary>
/// <param name="viewProjectionMatrix">ビューx射影行列</param>
/// <param name="viewportMatrix">ビューポート行列</param>
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4 viewportMatrix) {
	const float kGridHalfWidth = 2.0f;	// 半分の幅
	const uint32_t kSubdivision = 10;	// 分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);	// 1つ分の長さ
	Matrix4x4 screenTransformMatrix = Multiply(viewProjectionMatrix, viewportMatrix);

	// 奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		// ワールド座標系上の始点と終点を求める
		Vector3 start = { -kGridHalfWidth + kGridEvery * xIndex, 0.0f, -kGridHalfWidth };
		Vector3 end = { -kGridHalfWidth + kGridEvery * xIndex, 0.0f, kGridHalfWidth };
		// 座標変換
		Vector3 startScreen = TransformVector(start, screenTransformMatrix);
		Vector3 endScreen = TransformVector(end, screenTransformMatrix);
		// 変換した座標を使って表示
		Novice::DrawLine(int(startScreen.x), int(startScreen.y),
			int(endScreen.x), int(endScreen.y), 0xAAAAAAFF);
	}

	// 左から右への線
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		// ワールド座標系上の始点と終点を求める
		Vector3 start = { -kGridHalfWidth, 0.0f, -kGridHalfWidth + kGridEvery * zIndex };
		Vector3 end = { kGridHalfWidth, 0.0f, -kGridHalfWidth + kGridEvery * zIndex };
		// 座標変換
		Vector3 startScreen = TransformVector(start, screenTransformMatrix);
		Vector3 endScreen = TransformVector(end, screenTransformMatrix);
		// 変換した座標を使って表示
		Novice::DrawLine(int(startScreen.x), int(startScreen.y),
			int(endScreen.x), int(endScreen.y), 0xAAAAAAFF);
	}
}

/// <summary>
/// 球の描画
/// </summary>
/// <param name="sphere">球</param>
/// <param name="viewProjectionMatrix">ビューx射影行列</param>
/// <param name="viewportMatrix">ビューポート行列</param>
/// <param name="color">色</param>
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 20;						// 分割数
	const float kLonEvery = float(M_PI) * 2.0f / kSubdivision;		// 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision;			// 緯度分割1つ分の角度
	Matrix4x4 screenTransformMatrix = Multiply(viewProjectionMatrix, viewportMatrix);

	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;	// 現在の緯度
		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;	// 現在の経度

			// world座標系でのa,b,cを求める
			Vector3 a, b, c;
			a = Add(sphere.center,
				Multiply(sphere.radius,
					{ cos(lat) * cos(lon), sin(lat), cos(lat) * sin(lon) }));
			b = Add(sphere.center,
				Multiply(sphere.radius,
					{ cos(lat + kLatEvery) * cos(lon), sin(lat + kLatEvery), cos(lat + kLatEvery) * sin(lon) }));
			c = Add(sphere.center,
				Multiply(sphere.radius,
					{ cos(lat) * cos(lon + kLonEvery), sin(lat), cos(lat) * sin(lon + kLonEvery) }));

			// a,b,cをScreen座標系に変換
			Vector3 aScreen = TransformVector(a, screenTransformMatrix);
			Vector3 bScreen = TransformVector(b, screenTransformMatrix);
			Vector3 cScreen = TransformVector(c, screenTransformMatrix);
			// 画面に描画
			Novice::DrawLine(int(aScreen.x), int(aScreen.y), int(bScreen.x), int(bScreen.y), color);
			Novice::DrawLine(int(aScreen.x), int(aScreen.y), int(cScreen.x), int(cScreen.y), color);
		}
	}
}
