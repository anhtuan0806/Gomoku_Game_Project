#include "UIComponents.h"

void DrawTextCentered(int y, int screenWidth, const char* text, int fontSize, Color color) {
    // Raylib cung cấp hàm MeasureText để đo độ dài của chữ (tính bằng pixel)
    int textWidth = MeasureText(text, fontSize);
    int x = (screenWidth - textWidth) / 2; // Công thức căn giữa cực chuẩn
    DrawText(text, x, y, fontSize, color);
}

void DrawTextNormal(int x, int y, const char* text, int fontSize, Color color) {
    DrawText(text, x, y, fontSize, color);
}

void DrawGridBoard(int startX, int startY, int size, int cellSize, Color lineColor) {
    int boardWidth = size * cellSize;
    int boardHeight = size * cellSize;

    // Vẽ khung viền bao quanh bàn cờ
    DrawRectangleLines(startX, startY, boardWidth, boardHeight, lineColor);

    // Vẽ các đường thẳng dọc và ngang để tạo thành lưới
    for (int i = 1; i < size; i++) {
        // Vẽ đường dọc
        DrawLine(startX + i * cellSize, startY, startX + i * cellSize, startY + boardHeight, lineColor);
        // Vẽ đường ngang
        DrawLine(startX, startY + i * cellSize, startX + boardWidth, startY + i * cellSize, lineColor);
    }
}

void DrawPiece(int x, int y, int cellSize, int pieceType) {
    int centerX = x + cellSize / 2;
    int centerY = y + cellSize / 2;
    int radius = cellSize / 2 - 10; // Căn lề một chút để quân cờ không chạm viền

    if (pieceType == -1) {
        // Vẽ chữ X (2 đường chéo đỏ, độ dày 5 pixel)
        DrawLineEx({ (float)centerX - radius, (float)centerY - radius }, { (float)centerX + radius, (float)centerY + radius }, 5, RED);
        DrawLineEx({ (float)centerX - radius, (float)centerY + radius }, { (float)centerX + radius, (float)centerY - radius }, 5, RED);
    }
    else if (pieceType == 1) {
        // Vẽ chữ O (Vòng tròn xanh dương)
        DrawCircleLines(centerX, centerY, radius, BLUE);
        // Vẽ thêm một vòng tròn nhỏ hơn bên trong để tạo độ dày
        DrawCircleLines(centerX, centerY, radius - 1, BLUE);
        DrawCircleLines(centerX, centerY, radius - 2, BLUE);
    }
}

void DrawCursorHighlight(int x, int y, int cellSize, Color highlightColor) {
    // Vẽ một ô vuông trong suốt (nhìn thấu nền) để đánh dấu vị trí đang trỏ tới
    DrawRectangle(x, y, cellSize, cellSize, Fade(highlightColor, 0.5f));
}

void DrawPlayerAvatar(int x, int y, AvatarType avatar) {
    // Tạm thời dùng các hình khối có màu sắc khác nhau để đại diện cho Avatar.
    // Nâng cấp thì dùng tải file ảnh bằng hàm: DrawTexture(avatarImage, x, y, WHITE);
    int size = 50;
    switch (avatar) {
    case AVATAR_HERO:
        DrawRectangle(x, y, size, size, GOLD);
        DrawText("H", x + 15, y + 10, 30, BLACK); // Chữ H (Hero)
        break;
    case AVATAR_ROBOT:
        DrawRectangle(x, y, size, size, DARKGRAY);
        DrawCircle(x + size / 2, y + size / 2, 10, RED); // Mắt đỏ của Robot
        break;
    default:
        DrawRectangle(x, y, size, size, LIGHTGRAY);
        break;
    }
}