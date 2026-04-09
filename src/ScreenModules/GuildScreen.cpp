#include "GuildScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include <string>

void UpdateGuildScreen(ScreenState& currentState, WPARAM wParam) {
    if (wParam == VK_ESCAPE) {
        currentState = SCREEN_MENU;
    }
}

void RenderGuildScreen(HDC hdc, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    
    // 1. Nền sân vận động
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Lớp kính phản quang mờ
    Gdiplus::SolidBrush shadowBrush(GdipColour::SHADOW_PANEL);
    g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

    // 2. Banner Tiêu đề
    DrawPixelBanner(g, hdc, L"HƯỚNG DẪN CHIẾN THUẬT", screenWidth / 2, 80, 500, Colour::WHITE, Colour::ORANGE_NORMAL);

    // 3. Nội dung hướng dẫn (Sử dụng Glassmorphism Panel)
    Gdiplus::SolidBrush panelBrush(GdipColour::SHADOW_HEAVY);
    int panelW = screenWidth - 100;
    int panelH = screenHeight - 250;
    int panelX = 50;
    int panelY = 150;
    
    Gdiplus::Rect panelRect(panelX, panelY, panelW, panelH);
    g.FillRectangle(&panelBrush, panelRect);
    
    Gdiplus::Pen borderPen(GdipColour::GLASS_GLEAM, 2);
    g.DrawRectangle(&borderPen, panelRect);

    // 4. Vẽ các đề mục hướng dẫn
    int textY = panelY + 40;
    int spacing = 35;

    auto DrawHelpLine = [&](const std::wstring& title, const std::wstring& content, COLORREF color) {
        DrawTextCentered(hdc, title, textY, screenWidth, color, GlobalFont::Bold);
        textY += spacing;
        DrawTextCentered(hdc, content, textY, screenWidth, Colour::WHITE, GlobalFont::Normal);
        textY += spacing + 15;
    };

    DrawHelpLine(L"⚽ LUẬT CHƠI CARO (GOMOKU)", L"Đưa được 5 quân cờ liên tiếp theo hàng ngang, dọc hoặc chéo để ghi bàn thắng.", Colour::YELLOW_NORMAL);
    DrawHelpLine(L"🏟️ CHẾ ĐỘ TIC-TAC-TOE", L"Đưa được 3 quân cờ liên tiếp trên bàn cờ 3x3 để giành chiến thắng.", Colour::CYAN_NORMAL);
    DrawHelpLine(L"🎮 ĐIỀU KHIỂN", L"Dùng các phím Mũi tên hoặc W/A/S/D để di chuyển. Nhấn ENTER hoặc SPACE để đặt cờ.", Colour::ORANGE_LIGHT);
    DrawHelpLine(L"🏆 THỜI GIAN & ĐIỂM SỐ", L"Mỗi lượt có thời gian đếm ngược. Thắng đủ số trận (Target Score) để vô địch.", Colour::GREEN_LIGHT);

    // 5. Nút thoát
    int pulse = (int)(sin(g_GlobalAnimTime * 5.0f) * 50 + 205);
    COLORREF escColor = RGB(255, pulse, pulse);
    DrawTextCentered(hdc, L"Nhấn [ESC] để quay lại Menu chính", screenHeight - 60, screenWidth, escColor, GlobalFont::Note);
}
