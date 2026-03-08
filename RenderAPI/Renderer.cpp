#include "Renderer.h"

void InitGameWindow(uint16_t width, uint16_t height, const char* title) {
    // Khởi tạo cửa sổ với kích thước pixel
    InitWindow(width, height, title);

    // Khóa tốc độ khung hình ở 60 FPS 
    SetTargetFPS(60);
}

void CloseGameWindow() {
    CloseWindow();
}

bool IsWindowRunning() {
    // WindowShouldClose() trả về true nếu người dùng bấm [X] hoặc phím ESC
    return !WindowShouldClose();
}

void BeginRender() {
    BeginDrawing();
}

void EndRender() {
    EndDrawing();
}

void ClearScreenBackground(Color color) {
    // Xóa sạch khung hình cũ 
    ClearBackground(color);
}