#include "UIComponents.h"
#include "Colours.h"

void DrawGrid(HDC hdc, int size, int cellSize, int offsetX, int offsetY) {
    // 1. Khởi tạo và quản lý trạng thái GDI an toàn
    HPEN hPen = CreatePen(PS_SOLID, 1, Colour::GRAY_NORMAL);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // 2. Tiền tính toán các biểu thức bất biến trong vòng lặp 
    int boardLength = size * cellSize;

    // Khởi tạo các biến quy nạp
    int currentX = offsetX;
    int currentY = offsetY;

    // 3. Vòng lặp áp dụng Strength Reduction
    for (int i = 0; i <= size; ++i) { 
        // Vẽ đường ngang
        MoveToEx(hdc, offsetX, currentY, NULL);
        LineTo(hdc, offsetX + boardLength, currentY);

        // Vẽ đường dọc
        MoveToEx(hdc, currentX, offsetY, NULL);
        LineTo(hdc, currentX, offsetY + boardLength);

        // Phép cộng lũy kế 
        currentX += cellSize;
        currentY += cellSize;
    }

    // 4. Phục hồi trạng thái máy GDI và giải phóng tài nguyên
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void DrawHighlight(HDC hdc, int row, int col, int cellSize, int offX, int offY) {
    RECT rect = {
        offX + col * cellSize,
        offY + row * cellSize,
        offX + (col + 1) * cellSize,
        offY + (row + 1) * cellSize
    };

    HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
    // Dùng FrameRect để chỉ vẽ viền, không đè màu lên quân cờ bên dưới
    FrameRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

HPEN SetDrawColour(HDC hdc, COLORREF colour, int thickness) {
    HPEN hNewPen = CreatePen(PS_SOLID, thickness, colour);
    // Gắn bút mới vào DC và trả về bút mặc định/bút cũ của hệ thống
    return (HPEN)SelectObject(hdc, hNewPen);
}

void ResetDrawColour(HDC hdc, HPEN hOldPen) {
    // Phục hồi bút cũ, đồng thời lấy lại handle của bút mới vừa tạo
    HPEN hUsedPen = (HPEN)SelectObject(hdc, hOldPen);
    // An toàn xóa bút mới
    DeleteObject(hUsedPen);
}

void SetTextColour(HDC hdc, COLORREF colour) {
    ::SetTextColor(hdc, colour);
    SetBkMode(hdc, TRANSPARENT); // Giúp chữ không có nền màu bao quanh
}

void DrawSprite(HDC hdc, const Sprite& sprite, int x, int y, int targetSize) {
    if (!sprite.image) return;
    Gdiplus::Graphics g(hdc);
    DrawSprite(g, sprite, x, y, targetSize);
}

void DrawSprite(Gdiplus::Graphics& g, const Sprite& sprite, int x, int y, int targetSize) {
    if (!sprite.image) return;

    // Ảnh đã được scale chuẩn kích thước từ trước, ta chỉ dùng hàm DrawImage 1:1
    g.DrawImage(sprite.image, (Gdiplus::REAL)x, (Gdiplus::REAL)y);
}

void FreeSprite(Sprite& sprite) {
    if (sprite.image) {
        delete sprite.image;
        sprite.image = nullptr;
        sprite.width = 0;
        sprite.height = 0;
    }
}

void ScaleSprite(Sprite& sprite, int targetWidth, int targetHeight) {
    if (!sprite.image) return;

    // 1. Tạo bề mặt Bitmap mới trên RAM với kích thước đích chuẩn xác
    Gdiplus::Bitmap* scaledBitmap = new Gdiplus::Bitmap(targetWidth, targetHeight, PixelFormat32bppARGB);

    // 2. Mượn Gdiplus::Graphics để vẽ ảnh gốc lên Bitmap mới
    Gdiplus::Graphics g(scaledBitmap);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.DrawImage(sprite.image, 0, 0, targetWidth, targetHeight); // Chỉ scale 1 lần tại đây

    // 3. Giải phóng bộ nhớ ảnh gốc để tránh rò rỉ và cập nhật cấu trúc Sprite
    delete sprite.image;
    sprite.image = scaledBitmap;
    sprite.width = targetWidth;
    sprite.height = targetHeight;
}

void DrawTextCentered(HDC hdc, const std::wstring& text, int y, int screenWidth, COLORREF color, HFONT hFont) {
    // Nếu hàm gọi không cung cấp Font, tự động Fallback về Font mặc định toàn cục
    HFONT fontToUse = (hFont != nullptr) ? hFont : GlobalFont::Default;

    // Đẩy Font và màu vào Device Context
    HFONT hOldFont = (HFONT)SelectObject(hdc, fontToUse);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    // Xác định vùng vẽ chữ và gọi API
    // (Cho chiều cao rect dư dả 1 chút để không bị cắt chữ: y + 100)
    RECT rect = { 0, y, screenWidth, y + 100 };
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

    // Phục hồi trạng thái
    SelectObject(hdc, hOldFont);
}