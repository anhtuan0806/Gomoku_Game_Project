#include "Renderer.h"

namespace GlobalFont {
    HFONT Default = nullptr;
    HFONT Bold = nullptr;
    HFONT Title = nullptr;

    void Initialize() {
        Default = CreateFontW(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        Bold = CreateFontW(35, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        Title = CreateFontW(50, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    }

    void Cleanup() {
        if (Default) DeleteObject(Default);
        if (Bold) DeleteObject(Bold);
        if (Title) DeleteObject(Title);
    }
}

bool InitGraphics(ULONG_PTR& token) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    return Gdiplus::GdiplusStartup(&token, &gdiplusStartupInput, NULL) == Gdiplus::Ok;
}

void ShutdownGraphics(ULONG_PTR token) {
    Gdiplus::GdiplusShutdown(token);
}

Sprite LoadPNG(const wchar_t* path) {
    Sprite sprite = { nullptr, 0, 0 };
    Gdiplus::Image* img = Gdiplus::Image::FromFile(path);
    if (img != nullptr && img->GetLastStatus() == Gdiplus::Ok) {
        sprite.image = img;
        sprite.width = img->GetWidth();
        sprite.height = img->GetHeight();
    }
    return sprite;
}

void FreeSprite(Sprite& sprite) {
    if (sprite.image) {
        delete sprite.image;
        sprite.image = nullptr;
        sprite.width = sprite.height = 0;
    }
}

void ScaleSprite(Sprite& sprite, int targetWidth, int targetHeight) {
    if (!sprite.image) return;
    Gdiplus::Bitmap* scaledBitmap = new Gdiplus::Bitmap(targetWidth, targetHeight, PixelFormat32bppARGB);
    Gdiplus::Graphics g(scaledBitmap);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.DrawImage(sprite.image, 0, 0, targetWidth, targetHeight);

    delete sprite.image;
    sprite.image = scaledBitmap;
    sprite.width = targetWidth;
    sprite.height = targetHeight;
}

void CreateBuffer(HWND hwnd, HDC hdc, DoubleBuffer& buffer) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    buffer.width = rect.right - rect.left;
    buffer.height = rect.bottom - rect.top;

    buffer.hdcMem = CreateCompatibleDC(hdc);
    buffer.hBitmap = CreateCompatibleBitmap(hdc, buffer.width, buffer.height);
    buffer.hOldBitmap = (HBITMAP)SelectObject(buffer.hdcMem, buffer.hBitmap);

    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(buffer.hdcMem, &rect, hBrush);
    DeleteObject(hBrush);
}

void DeleteBuffer(DoubleBuffer& buffer) {
    if (buffer.hdcMem) {
        SelectObject(buffer.hdcMem, buffer.hOldBitmap);
        DeleteObject(buffer.hBitmap);
        DeleteDC(buffer.hdcMem);
        buffer.hdcMem = nullptr;
    }
}