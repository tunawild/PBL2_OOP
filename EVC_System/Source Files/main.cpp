#include "raylib.h"

int main() {
    // Khởi tạo cửa sổ 800x450 với tiêu đề test
    InitWindow(800, 450, "Test Raylib - PBL2 EV Station");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Raylib dang hoat dong tot!", 190, 200, 30, LIGHTGRAY);
            DrawCircle(400, 300, 40, GREEN); // Vẽ 1 hình tròn màu xanh
        EndDrawing();
    }

    CloseWindow();
    return 0;
}