#include "game.h"
#include "player.h"
#include "raylib.h"

int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,    
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

int main() {
    run();
    return 0;
}

void drawMap() {
    int width = 12;
    int height = 8;
    for (int i = 0; i < width * height; i++) {
        int tile = map[i];
        int x = i % width;
        int y = i / width;

        if (tile > 0) {
            DrawRectangle(x * 100.0f, y * 100.0f, 100.0f, 100.0f, RED);
        }
    }
}

void run() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hell Yeah");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        updatePlayer();
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        drawMap();
        drawPlayer();
        EndDrawing();
    }
    CloseWindow();
}
