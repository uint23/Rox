#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#include "beatmap_parser.hh"
#include "shapes.hh"

#define GAME_NAME "Rox - Aim Trainer"

using std::cout, std::endl, std::vector;

typedef enum {
	STATE_MENU,
	STATE_PLAYING,
	STATE_RESULTS,
} GameState;

typedef struct { /* beatmap */
	std::string  path;
	std::string  name;
	std::string  stars;
	OsuHitObject ho;
	/* ... */
} BMP;

struct {
	bool framerate = true;
	bool frametime = true;
	bool hitboxes  = false;
	bool showrays  = false;
} dbgopts;

/* draw on screen ui */
void draw_ui(void);

/* draw 3d world */
void draw_3d(void);

/* TODO change to a texture/bitmap of sorts */
void draw_crosshair(void);

/* draws items for menu state */
void draw_state_menu(void);

/* draws items for playing state */
void draw_state_playing(void);

/* draws items for results state */
void draw_state_results(void);

/* initialise default values for globals */
void init(void);

/* normalise OSU(x, y)->WorldScreen(x, y, z).
   takes (x, y), Cuboid screen to project to */
Vector3 normalise_osu_world(float px, float py, ShapeCuboid scr);

/* update camera on keypress W/A/S/D
  
   TODO rebinded wasd
   TODO normalise vectors */
void update_camera_wasd(void);

/* update targets on screen */
void update_targets(void);

/* change key configured options */
void update_options(void);

GameState      gamestate;
vector<BMP>    all_beatmaps;
vector<Target> all_targets;
vector<Line>   fired_shots;
ShapeCuboid    screen;
float fov = 60;
int scr_width  = 640;
int scr_height = 480;
Vector2 scr_center = {
	.x = (float)scr_width/2,
	.y = (float)scr_height/2,
};

Camera3D camera;
Ray      gun_ray;
Mesh     sphere_mesh;
Material sphere_material;

void draw_ui(void)
{
	draw_crosshair();
	if (dbgopts.framerate) {
		std::string fps = "FPS: " + std::to_string(GetFPS());
		DrawText(fps.c_str(), 10, 10, 20, RED);
	}
	if (dbgopts.frametime) {
		float ft = GetFrameTime()*1000.0f;
		std::string frametime = "FrameTime: " + std::to_string(ft).substr(0, 4) +
		                        "ms";
		DrawText(frametime.c_str(), 10, 10+20, 20, RED);
	}
}

void draw_3d(void)
{
	BeginMode3D(camera);

	/* targets */
	for (auto& i : all_targets) {
		if (!i.visible)
			continue;

		Matrix tfm = MatrixTranslate(i.pos.x, i.pos.y, i.pos.z);

		if (i.type == ShapeTypeSphere) {
			if (i.hit)
				sphere_material.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
			else
				sphere_material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
			DrawMesh(sphere_mesh, sphere_material, tfm);

			if (dbgopts.hitboxes) {
				float boxsize = i.shape.sphere.radius * 2.0f;
				DrawCubeWires(i.pos, boxsize, boxsize, boxsize, YELLOW);
			}
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
	if (dbgopts.showrays) {
		for (auto& i : fired_shots)
			DrawLine3D(i.start, i.end, BLUE);
	}

	/* playing screen */
	DrawCube({screen.x, screen.y, screen.z}, screen.width, screen.height, screen.length, BLACK);

	EndMode3D();
}

void draw_crosshair(void)
{
    int len = 5;
    int cx = scr_center.x;
    int cy = scr_center.y;

    DrawLine(cx, cy-len, cx, cy+len, BLACK); /* vertical */
    DrawLine(cx-len, cy, cx+len, cy, BLACK); /* horizontal */
}

void draw_state_menu(void)
{
	if (IsKeyPressed(KEY_ENTER)) {
		gamestate = STATE_PLAYING;
		DisableCursor(); 
	}

	BeginDrawing();
	ClearBackground(RAYWHITE);

	DrawText("Rox - Aim Trainer", 40, 100, 20, BLUE);
	DrawText("Press ENTER ", 40, 160, 20, BLUE);

	EndDrawing();
}

void draw_state_playing(void)
{
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

void draw_state_results(void)
{
}

void init(void)
{
	InitWindow(scr_width, scr_height, GAME_NAME);
	SetTargetFPS(144);

	camera = {
		.position = (Vector3){ 0.0f, 2.0f, 4.0f },
		.target = (Vector3){ 0.0f, 2.0f, 0.0f },
		.up = (Vector3){ 0.0f, 1.0f, 0.0f },
		.fovy = fov,
		.projection = CAMERA_PERSPECTIVE,
	};

	/* load beatmap */
	OsuBeatmap bmp;
	/* TODO change load beatmap to return OsuBeatmap */
	load_osu_beatmap(&bmp, "test.osu");

	screen = {
		.x = 0.0f, .y = 2.0f, .z = 0.0f,
		.width = 8.0f, .height = 6.0f, .length = 0.01f
	};

	/* create sphere mesh */
	sphere_mesh = GenMeshSphere(0.25f, 16, 16);
	sphere_material = LoadMaterialDefault();
	sphere_material.maps[MATERIAL_MAP_DIFFUSE].color = RED;

	/* insert all targets from beatmap */
	for (auto& i : bmp.objects) {
		Vector3 pos = normalise_osu_world(i.x, i.y, screen);
		all_targets.push_back(
			(Target) {
				.shape = { .sphere {.radius = 0.25f} },
				.type = ShapeTypeSphere,
				.pos = pos, .col = RED,
				.time_ms = i.time_ms,
				.visible = true, .hit = false,
			}
		);
	}

	gamestate = STATE_MENU;
}

Vector3 normalise_osu_world(float px, float py, ShapeCuboid scr)
{
	/* normalised (x, y):
	   (512 and 384 are the coordinate bounds used by osu)
	   [https://osu.ppy.sh/community/forums/topics/201854?n=1] */
	float nx = px / 512.0f;
	float ny = py / 384.0f;

	return (Vector3) {
		.x = scr.x - (scr.width * 0.5f) + (nx * scr.width),
		.y = scr.y + (scr.height * 0.5f) - (ny * scr.height),
		.z = scr.z,
	};
}

void update_camera_wasd(void)
{
	float step = GetFrameTime() * 10.0f;
	UpdateCameraPro(
		&camera,
		(Vector3) {
			(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))*step -
			(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))*step,
			(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))*step -
			(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))*step,
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
	/* debug options */
	if (IsKeyDown(KEY_RIGHT_SHIFT)) {
		if (IsKeyPressed(KEY_R))
			dbgopts.showrays = !dbgopts.showrays;
		if (IsKeyPressed(KEY_H))
			dbgopts.hitboxes = !dbgopts.hitboxes;
		if (IsKeyPressed(KEY_F))
			dbgopts.framerate = !dbgopts.framerate;
		else if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_F))
			dbgopts.frametime = !dbgopts.frametime;
	}
}

int main(int argc, char* argv[])
{
	if (argc > 1) {
		std::string av1 = std::string(argv[1]);

		if (av1 == "--test-parser") {
			OsuBeatmap test_bmp;
			load_osu_beatmap(&test_bmp, "test.osu");
			cout << test_bmp.audio_fp << endl;
			cout << "time\ttype\tx\ty" << endl;
			for (auto& i : test_bmp.objects) {
				cout << i.time_ms << "\t";
				cout << i.type << "\t";
				cout << i.x << "\t";
				cout << i.y << "\t" << std::endl;
			}
			return EXIT_SUCCESS;

		}

		else {
			std::cout << "Usage:" << std::endl;
			std::cout << "\t[--test-parser] outputs parser logs\n"
			<< std::endl;
			return EXIT_FAILURE;
		}
	}

	init();
	while (!WindowShouldClose()) {
		switch (gamestate) {
		case STATE_MENU:
			draw_state_menu();
			break;
		case STATE_PLAYING:
			draw_state_playing();
			break;
		case STATE_RESULTS:
			draw_state_results();
			break;
		}
	}
	UnloadMesh(sphere_mesh);
	UnloadMaterial(sphere_material);
	CloseWindow();
	return EXIT_SUCCESS;
}

