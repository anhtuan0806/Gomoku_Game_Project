#include "Renderer.h"
#include "UIScaler.h"
#include "Colours.h"

/** @file Renderer.cpp
 *  @brief Cài đặt cho quản lý font toàn cục và lifecycle của GDI+/double buffer.
 */

namespace GlobalFont
{
    HFONT Default = nullptr;
    HFONT Bold = nullptr;
    HFONT Title = nullptr;
    HFONT Note = nullptr;
}

// --- Globals ---
DoubleBuffer g_BackBuffer = {0};
ULONG_PTR g_GdiplusToken = 0;
bool g_NeedsRedraw = true;

namespace GlobalFont
{
    void Initialize()
    {
        // Nạp file Font từ thư mục Asset vào process (tạm thời) để CreateFontW nhận diện.
        // FR_PRIVATE đảm bảo font không được ghi vào registry hệ thống.
        AddFontResourceExW(L"Asset/font/VT323/VT323-Regular.ttf", FR_PRIVATE, 0);

        RebuildFonts();
    }

    void RebuildFonts()
    {
        // Xóa HFONT cũ (nếu có) trước khi tạo mới để tránh rò rỉ GDI.
        if (Default)
        {
            DeleteObject(Default);
            Default = nullptr;
        }
        if (Bold)
        {
            DeleteObject(Bold);
            Bold = nullptr;
        }
        if (Title)
        {
            DeleteObject(Title);
            Title = nullptr;
        }
        if (Note)
        {
            DeleteObject(Note);
            Note = nullptr;
        }

        // Tạo các font dựa trên UIScaler giúp UI co giãn theo kích thước màn hình.
        Default = CreateFontW(UIScaler::S(36), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");

        Bold = CreateFontW(UIScaler::S(42), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");

        Title = CreateFontW(UIScaler::S(64), 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");

        Note = CreateFontW(UIScaler::S(28), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
    }

    void Cleanup()
    {
        if (Default)
        {
            DeleteObject(Default);
            Default = nullptr;
        }
        if (Bold)
        {
            DeleteObject(Bold);
            Bold = nullptr;
        }
        if (Title)
        {
            DeleteObject(Title);
            Title = nullptr;
        }
        if (Note)
        {
            DeleteObject(Note);
            Note = nullptr;
        }

        // Gỡ font resource đã thêm bằng AddFontResourceExW.
        RemoveFontResourceExW(L"Asset/font/VT323/VT323-Regular.ttf", FR_PRIVATE, 0);
    }
}

bool InitGraphics(ULONG_PTR &gdiplusToken)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    return Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok;
}

void ShutdownGraphics(ULONG_PTR gdiplusToken)
{
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void CreateBuffer(HWND hwnd, HDC windowHdc, DoubleBuffer &buffer)
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    buffer.width = max(1, clientRect.right - clientRect.left);
    buffer.height = max(1, clientRect.bottom - clientRect.top);

    buffer.hdcMemory = CreateCompatibleDC(windowHdc);
    buffer.hBitmap = CreateCompatibleBitmap(windowHdc, buffer.width, buffer.height);
    buffer.hOldBitmap = (HBITMAP)SelectObject(buffer.hdcMemory, buffer.hBitmap);

    // Khởi tạo nền trắng an toàn
    HBRUSH backgroundBrush = CreateSolidBrush(ToCOLORREF(Palette::White));
    RECT fillRect = {0, 0, buffer.width, buffer.height};
    FillRect(buffer.hdcMemory, &fillRect, backgroundBrush);
    DeleteObject(backgroundBrush);
}

void DeleteBuffer(DoubleBuffer &buffer)
{
    if (buffer.hdcMemory)
    {
        SelectObject(buffer.hdcMemory, buffer.hOldBitmap);
        DeleteDC(buffer.hdcMemory);
        buffer.hdcMemory = NULL;
    }

    if (buffer.hBitmap)
    {
        DeleteObject(buffer.hBitmap);
        buffer.hBitmap = NULL;
    }
}