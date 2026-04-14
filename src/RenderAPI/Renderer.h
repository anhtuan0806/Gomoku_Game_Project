#ifndef _RENDERER_H_
#define _RENDERER_H_
#include <windows.h>
#include <gdiplus.h>
#include <string>
#pragma comment(lib, "gdiplus.lib")

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
    extern HFONT Note;

    void Initialize();
    void RebuildFonts();
    void Cleanup();
}

// Khởi tạo và tắt GDI+
bool InitGraphics(ULONG_PTR& token);
void ShutdownGraphics(ULONG_PTR token);

// Quản lý bộ đệm (Buffer)
void CreateBuffer(HWND hwnd, HDC hdc, DoubleBuffer& buffer);
void DeleteBuffer(DoubleBuffer& buffer);

#endif // _RENDERER_H_