#include "Renderer.h"
#include "UIScaler.h"

namespace GlobalFont
{
    HFONT Default = nullptr;
    HFONT Bold = nullptr;
    HFONT Title = nullptr;
    HFONT Note = nullptr;

    void Initialize()
    {
        // Nạp file Font từ thư mục Asset vào môi trường chạy tạm thời của Windows
        AddFontResourceExW(L"Asset/font/VT323/VT323-Regular.ttf", FR_PRIVATE, 0);

        RebuildFonts();
    }

    void RebuildFonts()
    {
        if (Default)
        {
            DeleteObject(Default);
        }
        if (Bold)
        {
            DeleteObject(Bold);
        }
        if (Title)
        {
            DeleteObject(Title);
        }
        if (Note)
        {
            DeleteObject(Note);
        }

        Default = CreateFontW(UIScaler::S(36), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
        Bold = CreateFontW(UIScaler::S(42), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
        Title = CreateFontW(UIScaler::S(64), 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
        Note = CreateFontW(UIScaler::S(28), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
    }

    void Cleanup()
    {
        if (Default)
        {
            DeleteObject(Default);
        }
        if (Bold)
        {
            DeleteObject(Bold);
        }
        if (Title)
        {
            DeleteObject(Title);
        }
        if (Note)
        {
            DeleteObject(Note);
        }

        // Gỡ bỏ cấu trúc Font khỏi RAM của Hệ Độ Hành
        RemoveFontResourceExW(L"Asset/font/VT323/VT323-Regular.ttf", FR_PRIVATE, 0);
    }
}

bool InitGraphics(ULONG_PTR &token)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    return Gdiplus::GdiplusStartup(&token, &gdiplusStartupInput, NULL) == Gdiplus::Ok;
}

void ShutdownGraphics(ULONG_PTR token)
{
    Gdiplus::GdiplusShutdown(token);
}

void CreateBuffer(HWND hwnd, HDC hdc, DoubleBuffer &buffer)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Đảm bảo kích thước tối thiểu là 1x1 để CreateCompatibleBitmap không thất bại (NULL)
    if (width <= 0)
    {
        width = 1;
    }
    if (height <= 0)
    {
        height = 1;
    }
    buffer.width = width;
    buffer.height = height;

    buffer.hdcMem = CreateCompatibleDC(hdc);
    buffer.hBitmap = CreateCompatibleBitmap(hdc, width, height);
    buffer.hOldBitmap = (HBITMAP)SelectObject(buffer.hdcMem, buffer.hBitmap);

    // Xóa nền buffer trắng
    RECT fillRect = {0, 0, width, height};
    HBRUSH fillBrushLocal = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(buffer.hdcMem, &fillRect, fillBrushLocal);
    DeleteObject(fillBrushLocal);
}

void DeleteBuffer(DoubleBuffer &buffer)
{
    if (buffer.hdcMem)
    {
        SelectObject(buffer.hdcMem, buffer.hOldBitmap);
        DeleteObject(buffer.hBitmap);
        DeleteDC(buffer.hdcMem);
        buffer.hdcMem = nullptr;
    }
}