#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <string>
#pragma comment(lib, "gdiplus.lib")

// Đại diện cho hình ảnh (ảnh quân cờ X, O...)
struct Sprite {
    Gdiplus::Image* image;
    int width;
    int height;
};

// Quản lý bộ đệm kép (Double Buffering) chống nhấp nháy màn hình
struct DoubleBuffer {
    HDC hdcMem;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    int width, height;
};

// Quản lý Font chữ toàn cục
namespace GlobalFont {
    extern HFONT Default;
    extern HFONT Bold;
    extern HFONT Title;

    void Initialize();
    void Cleanup();
}

// Khởi tạo và tắt GDI+
bool InitGraphics(ULONG_PTR& token);
void ShutdownGraphics(ULONG_PTR token);

// Quản lý ảnh (Sprite)
Sprite LoadPNG(const wchar_t* path);
void FreeSprite(Sprite& sprite);
void ScaleSprite(Sprite& sprite, int targetWidth, int targetHeight);

// Quản lý bộ đệm (Buffer)
void CreateBuffer(HWND hwnd, HDC hdc, DoubleBuffer& buffer);
void DeleteBuffer(DoubleBuffer& buffer);