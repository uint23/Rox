#include <raylib.h>

Camera3D camera;

int main(void)
{
	InitWindow(640, 480, "Rhythm Game");

	while (!WindowShouldClose()) {
		BeginDrawing();
		BeginMode3D(camera);

		// ...

		EndMode3D();
		EndDrawing();
	}
}

