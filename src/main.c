#include "raylib.h"
#include "raymath.h"
#include "raylib.h"
#include <math.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

#define PLAYER_RADIUS 14.0f
#define MOVE_SPEED    300.0f

#define WEAPON_LENGTH 26.0f
#define WEAPON_WIDTH  6.0f

#define MAX_BULLETS   256
#define BULLET_SPEED  800.0f
#define BULLET_RADIUS 4.0f
#define BULLET_LIFE   1.5f

typedef struct Player {
	Vector2 pos;
	float aim_rotation;
} Player;

typedef struct Bullet {
	Vector2 pos;
	Vector2 vel;
	float life;
	bool active;
} Bullet;

int main(void) {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Top-Down Shooter Prototype");
	SetTargetFPS(60);

	Player player = {
		.pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f },
		.aim_rotation = 0.0f
	};

	Bullet bullets[MAX_BULLETS] = { 0 };

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();

		Vector2 move = { 0 };

		if (IsKeyDown(KEY_W)) move.y -= 1.0f;
		if (IsKeyDown(KEY_S)) move.y += 1.0f;
		if (IsKeyDown(KEY_A)) move.x -= 1.0f;
		if (IsKeyDown(KEY_D)) move.x += 1.0f;

		if (Vector2Length(move) > 0.0f)
			move = Vector2Normalize(move);

		player.pos.x += move.x * MOVE_SPEED * dt;
		player.pos.y += move.y * MOVE_SPEED * dt;

		Vector2 mouse = GetMousePosition();
		Vector2 aim_dir = Vector2Subtract(mouse, player.pos);

		if (Vector2Length(aim_dir) > 0.0f)
			player.aim_rotation = atan2f(aim_dir.y, aim_dir.x) * RAD2DEG;

		Vector2 aim_norm = Vector2Normalize(aim_dir);

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			for (int i = 0; i < MAX_BULLETS; i++)
			{
				if (!bullets[i].active)
				{
					Vector2 muzzle = {
						player.pos.x + aim_norm.x * WEAPON_LENGTH,
						player.pos.y + aim_norm.y * WEAPON_LENGTH
					};

					bullets[i].pos = muzzle;
					bullets[i].vel = Vector2Scale(aim_norm, BULLET_SPEED);
					bullets[i].life = BULLET_LIFE;
					bullets[i].active = true;
					break;
				}
			}
		}

		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (!bullets[i].active) continue;

			bullets[i].pos.x += bullets[i].vel.x * dt;
			bullets[i].pos.y += bullets[i].vel.y * dt;
			bullets[i].life -= dt;

			if (bullets[i].life <= 0.0f)
				bullets[i].active = false;
		}

		BeginDrawing();
		ClearBackground((Color){ 20, 20, 20, 255 });

		DrawCircleV(player.pos, PLAYER_RADIUS, RED);

		Rectangle weapon = {
			player.pos.x,
			player.pos.y - WEAPON_WIDTH / 2.0f,
			WEAPON_LENGTH,
			WEAPON_WIDTH
		};

		Vector2 origin = { 0.0f, WEAPON_WIDTH / 2.0f };
		DrawRectanglePro(weapon, origin, player.aim_rotation, BLACK);

		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (bullets[i].active)
				DrawCircleV(bullets[i].pos, BULLET_RADIUS, YELLOW);
		}

		DrawText("WASD move | Mouse aim | LMB shoot", 10, 10, 20, RAYWHITE);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}


