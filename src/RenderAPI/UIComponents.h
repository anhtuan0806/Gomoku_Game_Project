#pragma once
#include "Renderer.h"
#include "../ApplicationTypes/PlayState.h" // Nhúng PlayState để vẽ trực tiếp

// Vẽ một Sprite cụ thể lên màn hình (Bắt buộc dùng Gdiplus::Graphics để tối ưu)
void DrawSprite(Gdiplus::Graphics& g, const Sprite& sprite, int x, int y);

// Vẽ văn bản căn giữa
void DrawTextCentered(HDC hdc, const std::wstring& text, int y, int screenWidth, COLORREF color, HFONT hFont = nullptr);

// --- HÀM TỔNG HỢP VẼ GAME ---
// Hàm này sẽ vẽ lưới, các quân cờ hiện có, và ô viền highlight đang chọn
void DrawGameBoard(HDC hdc, const PlayState* state, int cellSize, int offsetX, int offsetY, const Sprite& spriteX, const Sprite& spriteO);

// Cài đặt màu sắc cho văn bản và chế độ nền trong suốt
void SetTextColour(HDC hdc, COLORREF colour);