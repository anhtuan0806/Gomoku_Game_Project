#pragma once
#include "Renderer.h"
#include "../ApplicationTypes/PlayState.h"
#include <vector>
#include <string>
#include <map>
#include <windows.h>
#include <gdiplus.h>

// Thời gian cho Animation toàn cục
extern float g_GlobalAnimTime;

// Cấu trúc chứa dữ liệu Ma Trận Pixel (Modular)
struct PixelModel {
    int width = 0;
    int height = 0;
    std::vector<std::vector<int>> data;
    bool isLoaded = false;
};

// Hàm Load dữ liệu Pixel từ file txt bên ngoài (hỗ trợ đọc config)
PixelModel LoadPixelModel(const std::string& filePath);
// Vẽ mô hình Pixel với Palette động
void DrawPixelModel(Gdiplus::Graphics& g, const PixelModel& model, int cx, int cy, int pSize, const std::map<int, Gdiplus::Color>& palette);

// Vẽ một Sprite cụ thể lên màn hình có resize động
void DrawSprite(Gdiplus::Graphics& g, const Sprite& sprite, int x, int y, int width, int height);

// Vẽ văn bản căn giữa
void DrawTextCentered(HDC hdc, const std::wstring& text, int y, int screenWidth, COLORREF color, HFONT hFont = nullptr);

// Vẽ nền sân vận động bằng giải thuật thay cho ảnh PNG
void DrawProceduralStadium(Gdiplus::Graphics& g, int screenWidth, int screenHeight);

// Vẽ Avatar cầu thủ theo kiểu Pixel Art 8x8 từ ma trận số
void DrawPixelAvatar(Gdiplus::Graphics& g, int x, int y, int size, int avatarType);

// Vẽ hoạt họa Pixel chuyên dụng cho Khán Đài
void DrawPixelFootball(Gdiplus::Graphics& g, int cx, int cy, int size);
void DrawPixelTrophy(Gdiplus::Graphics& g, int cx, int cy, int size);

// Vẽ Banner Tiêu đề Pixel Art cho màn hình Menu/Settings/Load
// text: chuỗi ký tự đơn giản (ASCII), cx/cy: tâm banner, accent: màu nổi bật
void DrawPixelBanner(Gdiplus::Graphics& g, HDC hdc, const std::wstring& text,
    int cx, int cy, int panelW, COLORREF textColor, COLORREF glowColor);

// --- HÀM TỔNG HỢP VẼ GAME ---
// Hàm này sẽ vẽ lưới, các quân cờ hiện có, và ô viền highlight đang chọn
void DrawGameBoard(HDC hdc, const PlayState* state, int cellSize, int offsetX, int offsetY, const Sprite& spriteX, const Sprite& spriteO);

// Cài đặt màu sắc cho văn bản và chế độ nền trong suốt
// Scale ảnh trước và lưu vào mảng
void PreScaleSprite(const Sprite& orig, Sprite& scaled, int width, int height);

void SetTextColour(HDC hdc, COLORREF colour);