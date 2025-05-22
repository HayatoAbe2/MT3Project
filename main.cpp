#include <Novice.h>
#include "Matrix4x4.h"
#define _USE_MATH_DEFINES
#include <math.h>
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

Vector3 Project(const Vector3& v1, const  Vector3& v2);
Vector3 ClosestPoint(const Vector3& point, const Segment& segment);
bool CheckCollision(const Sphere& sphere, const Plane& plane);
bool CheckCollision(const Segment& segment, const Plane& plane);
bool CheckCollision(const Line& line, const Plane& plane);
bool CheckCollision(const Ray& ray, const Plane& plane);
bool CheckCollision(const Triangle& triangle, const Segment& segment);
bool CheckCollision(const Triangle& triangle, const Line& line);
bool CheckCollision(const Triangle& triangle, const Ray& ray);
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

	Triangle triangle;
	triangle.vertices[0] = { -1.0f, 0.0f, 0.0f };
	triangle.vertices[1] = { 1.0f, 0.0f, 0.0f };
	triangle.vertices[2] = { 0.0f, 1.0f, 0.0f };
	Segment segment;
	segment.origin = { -2.0f,-1.0f,0.0f };
	segment.diff = { 3.0f,2.0f,2.0f };
	bool isCollision = false;

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

		isCollision = CheckCollision(triangle,segment);

		ImGui::Begin("Window");
		ImGui::DragFloat3("Triangle.v0", &triangle.vertices[0].x, 0.01f);
		ImGui::DragFloat3("Triangle.v1", &triangle.vertices[1].x, 0.01f);
		ImGui::DragFloat3("Triangle.v2", &triangle.vertices[2].x, 0.01f);
		ImGui::DragFloat3("Segment.origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat("Segment.diff", &segment.diff.x, 0.01f);
		

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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// ビューx射影行列の作成
		viewProjectionMatrix = MakeViewProjectionMatrix(cameraTransform, float(kWindowWidth) / float(kWindowHeight));

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 平面の描画
		DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, WHITE);

		// 線分の描画
		Vector3 start = TransformVector(TransformVector(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = TransformVector(TransformVector(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);		
		if (isCollision) {
			Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), RED);
		} else {
			Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);
		}



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
bool CheckCollision(const Segment& segment, const Plane& plane){
	// 法線と線の内積
	float dot = Dot(plane.normal, segment.diff);
	if (dot == 0.0f) { return false; } // 平行な場合衝突しない

	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;
	if (0 <= t  && t <= 1) { // 線分の範囲内なら衝突
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

// 三角形と線の衝突判定
bool CheckCollision(const Triangle& triangle, const Segment& segment){
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

bool CheckCollision(const Triangle& triangle, const Line& line)
{
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

bool CheckCollision(const Triangle& triangle, const Ray& ray)
{
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

// 垂直ベクトルを求める
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

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

void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color){
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
