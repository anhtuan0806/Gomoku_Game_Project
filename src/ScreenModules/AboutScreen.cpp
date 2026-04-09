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

    // Lớp kính trắng mờ (Light Mode style)
    Gdiplus::SolidBrush whiteGlassBrush(Gdiplus::Color(60, 255, 255, 255));
    g.FillRectangle(&whiteGlassBrush, 0, 0, screenWidth, screenHeight);

    // 2. Banner Tiêu đề
    DrawPixelBanner(g, hdc, L"GIỚI THIỆU", screenWidth / 2, 80, 500, Colour::WHITE, Colour::CYAN_NORMAL);

    // 3. Logo & Tên Game
    int trophySize = 60;
    DrawPixelTrophy(g, screenWidth / 2, 165, trophySize);
    
    DrawTextCentered(hdc, L"CARO CHAMPIONS LEAGUE", 205, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);
    DrawTextCentered(hdc, L"Version 1.0 (Extreme Edition)", 250, screenWidth, Colour::GRAY_DARK, GlobalFont::Note);

    // 4. Thông tin phát triển (Light Glassmorphism Panel)
    int panelW = 700;
    int panelH = 420;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = 280;

    Gdiplus::SolidBrush whitePanel(GdipColour::GLASS_WHITE);
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(GdipColour::PANEL_ORANGE_BORDER, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề Pixel Banner (Chỉ giữ lại viền và icon làm vật trang trí)
    DrawPixelBanner(g, hdc, L"", screenWidth / 2, panelY + 40, 
        panelW - 20, Colour::WHITE, RGB(255, 120, 0), "Asset/models/history_pitch.txt");

    // 4. Nội dung About
    SetBkMode(hdc, TRANSPARENT);
    DrawTextCentered(hdc, L"THỰC HIỆN BỞI NHÓM 3 - 25CTT6", panelY + 80, screenWidth, Colour::GRAY_DARKEST, GlobalFont::Bold);

    int rowH = 35;
    int tableW = 600;
    int tableH = 175; 
    int tableX = (screenWidth - tableW) / 2;
    int tableY = panelY + 115;
    int col1W = 380;

    // Header Background (Màu xanh nhạt cho tiêu đề bảng)
    Gdiplus::SolidBrush headerBg(Gdiplus::Color(60, 0, 150, 255));
    g.FillRectangle(&headerBg, tableX, tableY, tableW, rowH);

    // Vẽ Grid Lines (Màu tối cho nền sáng)
    Gdiplus::Pen gridPen(Gdiplus::Color(100, 100, 100, 100), 1.0f);
    for (int i = 0; i <= 5; i++) {
        g.DrawLine(&gridPen, (INT)tableX, (INT)(tableY + i * rowH), (INT)(tableX + tableW), (INT)(tableY + i * rowH));
    }
    g.DrawLine(&gridPen, (INT)tableX, (INT)tableY, (INT)tableX, (INT)(tableY + tableH)); 
    g.DrawLine(&gridPen, (INT)(tableX + col1W), (INT)tableY, (INT)(tableX + col1W), (INT)(tableY + tableH)); 
    g.DrawLine(&gridPen, (INT)(tableX + tableW), (INT)tableY, (INT)(tableX + tableW), (INT)(tableY + tableH)); 

    // Header Text
    HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, Colour::WHITE);
    RECT rHeader1 = { tableX, tableY, tableX + col1W, tableY + rowH };
    RECT rHeader2 = { tableX + col1W, tableY, tableX + tableW, tableY + rowH };
    DrawTextW(hdc, L"THÀNH VIÊN", -1, &rHeader1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hdc, L"MSSV", -1, &rHeader2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Body Text
    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, Colour::GRAY_DARKEST);
    struct Member { std::wstring name; std::wstring ms; };
    Member list[] = {
        { L"Trương Tuấn Anh", L"24120260" },
        { L"Lê Văn Quốc", L"24120421" },
        { L"Đặng Quang Sang", L"24120428" },
        { L"Trương Đoàn Công Thành", L"24120451" }
    };

    for (int i = 0; i < 4; i++) {
        RECT rName = { tableX + 20, tableY + (i + 1) * rowH, tableX + col1W - 5, tableY + (i + 2) * rowH };
        RECT rMs = { tableX + col1W, tableY + (i + 1) * rowH, tableX + tableW, tableY + (i + 2) * rowH };
        DrawTextW(hdc, list[i].name.c_str(), -1, &rName, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        DrawTextW(hdc, list[i].ms.c_str(), -1, &rMs, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(hdc, oldF);

    // Giảng viên hướng dẫn & Công nghệ
    int lineY = tableY + tableH + 20;
    DrawTextCentered(hdc, L"GIẢNG VIÊN HƯỚNG DẪN: Trương Toàn Thịnh", lineY, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Bold);

    lineY += 35;
    DrawTextCentered(hdc, L"CÔNG NGHỆ SỬ DỤNG", lineY, screenWidth, Colour::GRAY_DARK, GlobalFont::Bold);
    lineY += 28;
    DrawTextCentered(hdc, L"C++ / Win32 API / GDI+ (Raster & Procedural)", lineY, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);

    // 5. Nút thoát
    int pulse = (int)(sin(g_GlobalAnimTime * 4.0f) * 30 + 100);
    COLORREF escColor = RGB(pulse, 0, 0); // Nhấp nháy Đỏ thẫm trên nền sáng
    DrawTextCentered(hdc, L"Nhấn [ESC] để quay lại Menu chính", screenHeight - 60, screenWidth, escColor, GlobalFont::Note);
}
