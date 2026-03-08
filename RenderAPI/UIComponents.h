#pragma once
#include <raylib.h>
#include <string>
#include "../ApplicationTypes/PlayState.h"

// --- CÁC HÀM VẼ GIAO DIỆN 2D ---

// Vẽ dòng chữ căn giữa màn hình theo chiều ngang
void DrawTextCentered(int y, int screenWidth, const char* text, int fontSize, Color color);

// Vẽ chữ ở tọa độ bình thường
void DrawTextNormal(int x, int y, const char* text, int fontSize, Color color);

// Vẽ bàn cờ dạng lưới vuông (tính bằng pixel)
void DrawGridBoard(int startX, int startY, int size, int cellSize, Color lineColor);

// Vẽ quân cờ (X hoặc O) vào một ô cụ thể
void DrawPiece(int x, int y, int cellSize, int pieceType);

// Vẽ khung nhấp nháy (con trỏ chuột khi người chơi dùng W,A,S,D)
void DrawCursorHighlight(int x, int y, int cellSize, Color highlightColor);

// Vẽ Avatar nhân vật (Sử dụng các hình khối hoặc biểu tượng)
void DrawPlayerAvatar(int x, int y, AvatarType avatar);