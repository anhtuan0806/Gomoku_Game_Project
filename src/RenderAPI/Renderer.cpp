#include "Renderer.h"

namespace GlobalFont {
    HFONT Default = nullptr;
    HFONT Bold = nullptr;
    HFONT Title = nullptr;
    HFONT Note = nullptr;

    void Initialize() {
        // Nạp file Font từ thư mục Asset vào môi trường chạy tạm thời của Windows
        AddFontResourceExW(L"Asset/font/VT323/VT323-Regular.ttf", FR_PRIVATE, 0);

        Default = CreateFontW(36, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
        Bold = CreateFontW(42, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
        Title = CreateFontW(64, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
        Note = CreateFontW(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"VT323");
    }

    void Cleanup() {
        if (Default) DeleteObject(Default);
        if (Bold) DeleteObject(Bold);
        if (Title) DeleteObject(Title);
        if (Note) DeleteObject(Note);

        // Gỡ bỏ cấu trúc Font khỏi RAM của Hệ Độ Hành
        RemoveFontResourceExW(L"Asset/font/VT323/VT323-Regular.ttf", FR_PRIVATE, 0);
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