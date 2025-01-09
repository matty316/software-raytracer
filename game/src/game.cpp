#include "game.h"
#include "raylib.h"

int main() {
    run();
    return 0;
}

void run() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hell Yeah");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        EndDrawing();
    }
    CloseWindow();
}
