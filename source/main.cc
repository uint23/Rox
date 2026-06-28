#include <cstdlib>
#include <iostream>
#include <vector>

#include <raylib.h>

#include "shapes.h"

#define GAME_NAME "Rox - Aim Trainer"

using std::cout, std::endl;

/* draw on screen ui */
void draw_ui(void);

/* draw 3d world */
void draw_3d(void);

/* TODO change to a texture/bitmap of sorts */
void draw_crosshair(void);

/* initialise default values for globals */
void init(void);

/* update camera on keypress W/A/S/D
  
   TODO rebinded wasd
   TODO normalise vectors */
void update_camera_wasd(void);

std::vector<Target> all_targets;
int scr_width  = 640;
int scr_height = 480;
Vector2 scr_center = {
	.x = (float)scr_width/2,
	.y = (float)scr_height/2,
};

Camera3D camera;
Ray gun_ray;

void draw_ui(void)
{
	draw_crosshair();
}

void draw_3d(void)
{
	BeginMode3D(camera);

	/* walls */
	DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY);
	DrawCube((Vector3){ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);
	DrawCube((Vector3){ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);
	DrawCube((Vector3){ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);

	/* all targets */
	for (auto& i : all_targets) {
		if (i.type == ShapeTypeSphere) {
			DrawSphere(
				i.pos,
				i.shape.sphere.radius,
				i.col
			);
		}
		else if (i.type == ShapeTypeCapsule) {
			/* TODO */
		}
		else if (i.type == ShapeTypeCuboid) {
			/* TODO */
		}
		else if (i.type == ShapeTypeCube) {
			/* TODO */
		}
		else {
			cout << "Invalid Shape" << endl;
		}
	}

	EndMode3D();
}

void draw_crosshair(void)
{
	int radius = 4;
	DrawCircle(scr_center.x-radius, scr_center.y-radius, 4, WHITE);
}

void init(void)
{
	InitWindow(scr_width, scr_height, GAME_NAME);
	SetTargetFPS(144);
	DisableCursor();

	camera = {
		.position = (Vector3){ 0.0f, 2.0f, 4.0f },
		.target = (Vector3){ 0.0f, 2.0f, 0.0f },
		.up = (Vector3){ 0.0f, 1.0f, 0.0f },
		.fovy = 60.0f,
		.projection = CAMERA_PERSPECTIVE,
	};

	/* test target */
	for (float i = 1.0f; i < 4.0f; i+=0.5f) {
		all_targets.push_back(
			(Target) {
				.shape = { .sphere {.radius = 0.25f}, },
				.type = ShapeTypeSphere,
				.pos = {i, 1.0f, 1.0f},
				.col = RED,
			}
		);
	}
}

void update_camera_wasd(void)
{
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
}

int main(void)
{
	init();
	while (!WindowShouldClose()) {
		/* updates */
		update_camera_wasd();

		/* drawing */
		BeginDrawing();
		ClearBackground(WHITE);

		draw_3d();
		draw_ui();

		EndDrawing();
	}
	CloseWindow();
	return EXIT_SUCCESS;
}

