#include "player.h"
#include "raylib.h"

#define SPEED 5.f

struct Player {
	Vector2 position = { .x = 0.0f, .y = 0.0f };
	Vector2 size = { .x = 20.0f, .y = 20.0f };
} player;

void updatePlayer() {
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
		player.position.x -= SPEED;
	}
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
		player.position.x += SPEED;
	}
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
		player.position.y -= SPEED;
	}
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
		player.position.y += SPEED;
	}
}

void drawPlayer() {
	DrawRectangleV(player.position, player.size, BLUE);
}