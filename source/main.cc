#include <stdlib.h>

#include <raylib.h>

Camera3D camera;

int main(void)
{
	InitWindow(640, 480, "Rhythm Game");
	SetTargetFPS(144);
	DisableCursor();

	(Camera3D) camera = {
		.position = (Vector3){ 0.0f, 2.0f, 4.0f },
		.target = (Vector3){ 0.0f, 2.0f, 0.0f },
		.up = (Vector3){ 0.0f, 1.0f, 0.0f },
		.fovy = 60.0f,
		.projection = CAMERA_PERSPECTIVE,
	};


	while (!WindowShouldClose()) {
		UpdateCameraPro(
			&camera,
			(Vector3) {
				(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))*0.1f -
				(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))*0.1f,
				(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))*0.1f -
				(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))*0.1f,
				0.0f
			},
			(Vector3){GetMouseDelta().x*0.05f, GetMouseDelta().y*0.05f, 0.0f},
			GetMouseWheelMove()*2.0f
		);

		BeginDrawing();
		ClearBackground(WHITE);
		BeginMode3D(camera);

                DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY);
		DrawCube((Vector3){ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);
		DrawCube((Vector3){ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);
		DrawCube((Vector3){ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);

		EndMode3D();
		EndDrawing();
	}
	CloseWindow();
	return EXIT_SUCCESS;
}

