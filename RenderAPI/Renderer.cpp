#include "Renderer.h"

namespace GlobalFont {
    HFONT Default = nullptr;
    HFONT Bold = nullptr;
    HFONT Title = nullptr;

    void Initialize() {
        // Tham số cốt lõi: Chiều cao (Cỡ chữ), Độ đậm (FW_NORMAL/FW_BOLD), Tên Font (Segoe UI)
        Default = CreateFontW(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        Bold = CreateFontW(35, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        Title = CreateFontW(50, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    }

    void Cleanup() {
        if (Default) DeleteObject(Default);
        if (Bold) DeleteObject(Bold);
        if (Title) DeleteObject(Title);
    }
}

bool InitGraphics(ULONG_PTR& token) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    // Khởi tạo GDI+. Status::Ok tương ứng với giá trị 0.
    Gdiplus::Status status = Gdiplus::GdiplusStartup(&token, &gdiplusStartupInput, NULL);

    if (status == Gdiplus::Ok) {
        return true;
    }

    // Có thể log lỗi hoặc debug tại đây 
    return false;
}

void ShutdownGraphics(ULONG_PTR token) {
    // Kết thúc phiên làm việc của GDI+
    Gdiplus::GdiplusShutdown(token);
}

Sprite LoadPNG(const wchar_t* path) {
    Sprite sprite = { nullptr, 0, 0 };

    // Khởi tạo đối tượng Image từ đường dẫn Unicode
    Gdiplus::Image* img = Gdiplus::Image::FromFile(path);

    // Kiểm tra xem ảnh có được nạp thành công không
    if (img != nullptr && img->GetLastStatus() == Gdiplus::Ok) {
        sprite.image = img;
        sprite.width = img->GetWidth();
        sprite.height = img->GetHeight();
    }

    return sprite;
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