#include "UIComponents.h"
#include "Colours.h"

void DrawSprite(Gdiplus::Graphics& g, const Sprite& sprite, int x, int y) {
    if (sprite.image) {
        g.DrawImage(sprite.image, (Gdiplus::REAL)x, (Gdiplus::REAL)y);
    }
}

void DrawTextCentered(HDC hdc, const std::wstring& text, int y, int screenWidth, COLORREF color, HFONT hFont) {
    HFONT fontToUse = (hFont != nullptr) ? hFont : GlobalFont::Default;
    HFONT hOldFont = (HFONT)SelectObject(hdc, fontToUse);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    RECT rect = { 0, y, screenWidth, y + 100 };
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(hdc, hOldFont);
}

void DrawGameBoard(HDC hdc, const PlayState* state, int cellSize, int offsetX, int offsetY, const Sprite& spriteX, const Sprite& spriteO) {
    int size = state->boardSize;
    int boardLength = size * cellSize;

    // 1. Vẽ lưới (Grid)
    HPEN hPen = CreatePen(PS_SOLID, 1, Colour::GRAY_NORMAL);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (int i = 0; i <= size; ++i) {
        int currX = offsetX + i * cellSize;
        int currY = offsetY + i * cellSize;
        // Đường ngang
        MoveToEx(hdc, offsetX, currY, NULL);
        LineTo(hdc, offsetX + boardLength, currY);
        // Đường dọc
        MoveToEx(hdc, currX, offsetY, NULL);
        LineTo(hdc, currX, offsetY + boardLength);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // 2. Khởi tạo đối tượng đồ họa 1 LẦN DUY NHẤT để vẽ tất cả quân cờ
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            int drawX = offsetX + c * cellSize;
            int drawY = offsetY + r * cellSize;

            if (state->board[r][c] == CELL_PLAYER1) {
                DrawSprite(g, spriteX, drawX, drawY);
            }
            else if (state->board[r][c] == CELL_PLAYER2) {
                DrawSprite(g, spriteO, drawX, drawY);
            }
        }
    }

    // 3. Vẽ Highlight Con trỏ (Cursor)
    if (state->status == MATCH_PLAYING) {
        RECT cursorRect = {
            offsetX + state->cursorCol * cellSize,
            offsetY + state->cursorRow * cellSize,
            offsetX + (state->cursorCol + 1) * cellSize,
            offsetY + (state->cursorRow + 1) * cellSize
        };

        HBRUSH highlightBrush = CreateSolidBrush(Colour::RED_NORMAL);
        // Tăng độ dày viền highlight lên 3px để dễ nhìn hơn
        HPEN highlightPen = CreatePen(PS_SOLID, 3, Colour::RED_NORMAL);
        HPEN oldHighlightPen = (HPEN)SelectObject(hdc, highlightPen);

        // Vẽ viền thay vì lấp đầy
        FrameRect(hdc, &cursorRect, highlightBrush);

        SelectObject(hdc, oldHighlightPen);
        DeleteObject(highlightPen);
        DeleteObject(highlightBrush);
    }
}

void SetTextColour(HDC hdc, COLORREF colour) {
    ::SetTextColor(hdc, colour); // Gọi hàm chuẩn của Windows GDI
    SetBkMode(hdc, TRANSPARENT); // Đảm bảo chữ không có nền màu bao quanh
}