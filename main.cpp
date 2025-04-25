#include <Novice.h>
#include "Matrix4x4.h"
const char kWindowTitle[] = "MT3";

// 表示用の関数
static const int kColumnWidth = 60;
static const int kRowHeight = 20;
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label);
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// 変数の宣言
	Vector3 v1(1.2f, -3.9f, 2.5f);
	Vector3 v2(2.8f, 0.4f, -1.3f);
	Vector3 cross = Cross(v1, v2);

	Vector3 scale = { 1.0f,1.0f,1.0f };
	Vector3 rotate{};
	Vector3 translate{0.0f,0.0f,100.0f};

	Vector3 cameraPosition = {};
	int kWindowWidth = 1280;
	int kWindowHeight = 720;
	Vector3 kLocalVertices[3];
	kLocalVertices[0] = { -10.0f,-10.0f,0.0f };
	kLocalVertices[1] = { 0.0f, 10.0f,0.0f };
	kLocalVertices[2] = { 10.0f, -10.0f, 0.0f };

	Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix({1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, cameraPosition);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);
	Vector3 screenVertices[3];
	for (uint32_t i = 0; i < 3; ++i) {
		Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
		screenVertices[i] = Transform(ndcVertex, viewportMatrix);
	}


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

		// WASDで三角形を移動
		if (keys[DIK_S]) {
			translate.z -= 0.5f;
		}
		if (keys[DIK_W]) {
			translate.z += 0.5f;
		}
		if (keys[DIK_A]) {
			translate.x -= 0.5f;
		}
		if (keys[DIK_D]) {
			translate.x += 0.5f;
		}

		// 常時Y軸回転
		rotate.y += 0.04f;

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		Novice::ScreenPrintf(800, 50, "%f", sqrtf(powf(100.0f, 2.0f) + powf(100.0f, 2.0f)));
		VectorScreenPrintf(0, 0, cross, "Cross");

		worldMatrix = MakeAffineMatrix(scale, rotate, translate);
		cameraMatrix = MakeAffineMatrix(scale, { 0.0f,0.0f,0.0f }, cameraPosition);
		viewMatrix = Inverse(cameraMatrix);
		projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);
		for (uint32_t i = 0; i < 3; ++i) {
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}

		Novice::DrawTriangle(
			int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y),
			int(screenVertices[2].x), int(screenVertices[2].y), RED, kFillModeSolid
		);

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

void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(
				x + column * kColumnWidth, y + row * kRowHeight + kRowHeight, "%6.02f", matrix.m[row][column]);
		}
	}
}