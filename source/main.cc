#include <cstdlib>
#include <iostream>
#include <vector>

#include <raylib.h>

#include "shapes.hh"

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

/* update targets on screen */
void update_targets(void);

/* change key configured options */
void update_options(void);

std::vector<Target> all_targets;
std::vector<Line>   fired_shots;
bool                show_rays;
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

	/* targets */
	for (auto& i : all_targets) {
		if (!i.visible)
			continue;

		if (i.type == ShapeTypeSphere) {
			if (i.hit)
				i.col = BLUE;
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

	/* shot trails */
	if (show_rays) {
		for (auto& i : fired_shots)
			DrawLine3D(i.start, i.end, BLUE);
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
				.visible = true,
				.hit = false,
			}
		);
	}

	show_rays = false;
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

void update_targets(void)
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { /* shot */
		Ray ray = GetScreenToWorldRay({ (float)scr_width/2, (float)scr_height/2 }, camera);
		Vector3 end;

		/* check if ray hit a target */
		bool hit = false;
		for (auto& i : all_targets) {
			if (i.type == ShapeTypeSphere) {
				RayCollision c = GetRayCollisionSphere(ray, i.pos, i.shape.sphere.radius);
				if (c.hit) {
					hit = i.hit = true;
					end = c.point;
				}
			}

			/* TODO other shapes */
		}

		/* default: shoot ray 100u if it didnt
		   intersect any targets */
		if (!hit) {
			end = {
				ray.position.x + ray.direction.x * 100.0f,
				ray.position.y + ray.direction.y * 100.0f,
				ray.position.z + ray.direction.z * 100.0f
			};
		}

		ray.position.x+=0.0001f;
		ray.position.y+=0.0001f;
		ray.position.z+=0.0001f;
		fired_shots.push_back({.start=ray.position, .end=end});
	}
}

void update_options(void)
{
	if (IsKeyPressed(KEY_R))
		show_rays = !show_rays;
}

int main(void)
{
	init();
	while (!WindowShouldClose()) {
		/* updates */
		update_camera_wasd();
		update_targets();
		update_options();

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

