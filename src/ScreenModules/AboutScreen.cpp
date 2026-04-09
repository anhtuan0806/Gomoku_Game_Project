#include "AboutScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include <string>

void UpdateAboutScreen(ScreenState& currentState, WPARAM wParam) {
    if (wParam == VK_ESCAPE) {
        currentState = SCREEN_MENU;
    }
}

void RenderAboutScreen(HDC hdc, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    
    // 1. Nền sân vận động
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Lớp kính phản quang mờ
    Gdiplus::SolidBrush shadowBrush(GdipColour::SHADOW_PANEL);
    g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

    // 2. Banner Tiêu đề
    DrawPixelBanner(g, hdc, L"GIỚI THIỆU PHIÊN BẢN", screenWidth / 2, 80, 500, Colour::WHITE, Colour::CYAN_NORMAL);

    // 3. Logo & Tên Game
    int trophySize = 80;
    DrawPixelTrophy(g, screenWidth / 2, 180, trophySize);
    
    DrawTextCentered(hdc, L"CARO CHAMPIONS LEAGUE", 220, screenWidth, Colour::YELLOW_NORMAL, GlobalFont::Title);
    DrawTextCentered(hdc, L"Version 1.0 (Extreme Edition)", 265, screenWidth, Colour::GRAY_LIGHT, GlobalFont::Note);

    // 4. Thông tin phát triển
    int panelW = 600;
    int panelH = 260;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = 320;

    Gdiplus::SolidBrush panelBrush(GdipColour::SHADOW_HEAVY);
    g.FillRectangle(&panelBrush, panelX, panelY, panelW, panelH);
    
    Gdiplus::Pen borderPen(GdipColour::PANEL_BLUE_BORDER, 2);
    g.DrawRectangle(&borderPen, panelX, panelY, panelW, panelH);

    int lineY = panelY + 30;
    DrawTextCentered(hdc, L"PHÁT TRIỂN BỞI", lineY, screenWidth, Colour::WHITE, GlobalFont::Bold);
    lineY += 40;
    DrawTextCentered(hdc, L"Sinh viên: [Lê Anh Tuấn] - MSSV: 24120260", lineY, screenWidth, Colour::CYAN_LIGHT, GlobalFont::Bold);
    lineY += 50;
    
    DrawTextCentered(hdc, L"CÔNG NGHỆ SỬ DỤNG", lineY, screenWidth, Colour::WHITE, GlobalFont::Bold);
    lineY += 35;
    DrawTextCentered(hdc, L"Ngôn ngữ: C++ / Win32 API", lineY, screenWidth, Colour::GRAY_LIGHT, GlobalFont::Normal);
    lineY += 30;
    DrawTextCentered(hdc, L"Đồ họa: GDI+ (Raster/Procedural Rendering)", lineY, screenWidth, Colour::GRAY_LIGHT, GlobalFont::Normal);

    // 5. Nút thoát
    int pulse = (int)(sin(g_GlobalAnimTime * 4.0f) * 50 + 205);
    COLORREF escColor = RGB(pulse, 255, pulse);
    DrawTextCentered(hdc, L"Nhấn [ESC] để quay lại Menu chính", screenHeight - 60, screenWidth, escColor, GlobalFont::Note);
}
