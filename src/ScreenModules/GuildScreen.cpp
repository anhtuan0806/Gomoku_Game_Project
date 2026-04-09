#include "GuildScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include <string>

void UpdateGuildScreen(ScreenState& currentState, int& currentPage, WPARAM wParam) {
    if (wParam == VK_ESCAPE) {
        currentState = SCREEN_MENU;
        currentPage = 0; 
    }
    else if (wParam == VK_RIGHT) {
        if (currentPage < 2) currentPage++;
    }
    else if (wParam == VK_LEFT) {
        if (currentPage > 0) currentPage--;
    }
}

void RenderGuildScreen(HDC hdc, int screenWidth, int screenHeight, int currentPage) {
    Gdiplus::Graphics g(hdc);
    
    // 1. Nền sân vận động
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Lớp kính trắng mờ (Light Mode style)
    Gdiplus::SolidBrush whiteGlassBrush(Gdiplus::Color(60, 255, 255, 255));
    g.FillRectangle(&whiteGlassBrush, 0, 0, screenWidth, screenHeight);

    // 2. Banner Tiêu đề (Đổi màu theo trang)
    COLORREF bannerColors[] = { Colour::ORANGE_NORMAL, Colour::CYAN_NORMAL, Colour::GREEN_NORMAL };
    std::wstring titles[] = { L"CHẾ ĐỘ CHƠI", L"CÁCH ĐIỀU KHIỂN", L"CƠ CHẾ TRẬN ĐẤU" };
    DrawPixelBanner(g, hdc, titles[currentPage], screenWidth / 2, 70, 500, Colour::WHITE, bannerColors[currentPage]);

    // 3. Khung Panel nội dung (Light Mode)
    int panelW = screenWidth - 80;
    int panelH = 480;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = 135;
    
    // Các thông số chung cho cột (Cần thiết cho logic trang 1 & 2)
    int colW = panelW / 2;
    int colPadding = 35;

    Gdiplus::SolidBrush whitePanel(GdipColour::GLASS_WHITE);
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(GdipColour::PANEL_GOLD_BORDER, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề Pixel Banner (Chỉ giữ lại viền và icon làm vật trang trí)
    DrawPixelBanner(g, hdc, L"", screenWidth / 2, panelY + 40,
        panelW - 20, Colour::WHITE, RGB(255, 215, 0), "Asset/models/badge.txt");
    Gdiplus::Pen dividerPen(Gdiplus::Color(50, 0, 0, 0), 1); // Vạch tối mờ trên nền sáng

    if (currentPage == 0) { // TRANG 1: HAI CHẾ ĐỘ CHƠI
        DrawTextCentered(hdc, L"HÃY CHỌN KỊCH BẢN CHIẾN THUẬT CỦA BẠN", panelY + 85, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Bold);
        
        // Vẽ vạch ngăn đôi cột
        g.DrawLine(&dividerPen, (INT)(panelX + colW), (INT)(panelY + 120), (INT)(panelX + colW), (INT)(panelY + panelH - 40));

        // -- CỘT TRÁI: CARO (GOMOKU) --
        int curY = panelY + 135;
        DrawTextCentered(hdc, L"CARO - TRUYỀN THỐNG", curY, screenWidth - colW, Colour::ORANGE_NORMAL, GlobalFont::Bold);
        curY += 50;
        RECT rLeft = { panelX + colPadding, curY, panelX + colW - colPadding, panelY + panelH - 20 };
        SetTextColor(hdc, Colour::GRAY_DARKEST);
        SelectObject(hdc, GlobalFont::Default);
        std::wstring caroTxt = L"- Kích thước sân: 15x15 ô cực rộng.\n"
                               L"- Luật thắng: Đạt đủ 5 quân cờ liên tiếp theo hàng ngang, dọc hoặc chéo.\n"
                               L"- Cơ chế: Đây là cuộc đấu trí tầm xa, bạn cần tính toán đa bước để bẫy đối thủ.\n"
                               L"- Ghi bàn: Mỗi lần đạt chuỗi 5, bạn sẽ ghi 1 bàn thắng vào lưới đối phương.";
        DrawTextW(hdc, caroTxt.c_str(), -1, &rLeft, DT_LEFT | DT_WORDBREAK);

        // -- CỘT PHẢI: TIC-TAC-TOE --
        curY = panelY + 135;
        DrawTextCentered(hdc, L"TIC-TAC-TOE - TỐC ĐỘ", curY, screenWidth + colW, Colour::CYAN_NORMAL, GlobalFont::Bold);
        curY += 50;
        RECT rRight = { panelX + colW + colPadding, curY, panelX + panelW - colPadding, panelY + panelH - 20 };
        std::wstring tttTxt = L"- Kích thước sân: 3x3 ô nhỏ gọn.\n"
                              L"- Luật thắng: Chỉ cần 3 quân cờ liên tiếp.\n"
                              L"- Cơ chế: Trận đấu diễn ra cực nhanh, đòi hỏi phản xạ và chiếm lĩnh trung tâm ngay lập tức.\n"
                              L"- Thích hợp cho những trận đấu chớp nhoáng, phân định thắng bại tức thì.";
        DrawTextW(hdc, tttTxt.c_str(), -1, &rRight, DT_LEFT | DT_WORDBREAK);

        DrawPixelFootball(g, screenWidth / 2, panelY + panelH - 60, 50);
    }
    else if (currentPage == 1) { // TRANG 2: ĐIỀU KHIỂN & THAO TÁC
        DrawTextCentered(hdc, L"THAO TÁC LINH HOẠT ĐỂ LÀM CHỦ TRẬN ĐẤU", panelY + 85, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Bold);
        g.DrawLine(&dividerPen, (INT)(panelX + colW), (INT)(panelY + 120), (INT)(panelX + colW), (INT)(panelY + panelH - 40));

        // -- CỘT TRÁI: DI CHUYỂN --
        int curY = panelY + 135;
        DrawTextCentered(hdc, L"DI CHUYỂN TRÊN SÂN", curY, screenWidth - colW, Colour::CYAN_NORMAL, GlobalFont::Bold);
        curY += 60;
        DrawTextCentered(hdc, L"[Mũi tên] hoặc [WASD]", curY, screenWidth - colW, Colour::GRAY_DARKEST, GlobalFont::Default);
        curY += 35;
        DrawTextCentered(hdc, L"Di chuyển con trỏ chọn ô.", curY, screenWidth - colW, Colour::GRAY_DARK, GlobalFont::Note);
        curY += 60;
        DrawTextCentered(hdc, L"Nhấn giữ để di chuyển nhanh", curY, screenWidth - colW, Colour::BLUE_DARKEST, GlobalFont::Default);

        // -- CỘT PHẢI: HÀNH ĐỘNG --
        curY = panelY + 135;
        DrawTextCentered(hdc, L"HÀNH ĐỘNG QUYẾT ĐỊNH", curY, screenWidth + colW, Colour::ORANGE_NORMAL, GlobalFont::Bold);
        curY += 60;
        DrawTextCentered(hdc, L"[ENTER] hoặc [SPACE]", curY, screenWidth + colW, Colour::GRAY_DARKEST, GlobalFont::Default);
        curY += 35;
        DrawTextCentered(hdc, L"Xác nhận đặt quân cờ.", curY, screenWidth + colW, Colour::GRAY_DARK, GlobalFont::Note);
        curY += 60;
        DrawTextCentered(hdc, L"[ESC]: Tạm dừng / Menu", curY, screenWidth + colW, Colour::BLUE_DARKEST, GlobalFont::Default);
        curY += 50;
        DrawTextCentered(hdc, L"[S]: Lưu trận đấu nhanh", curY, screenWidth + colW, Colour::BLUE_DARKEST, GlobalFont::Default);

        DrawPixelAvatar(g, screenWidth / 2 - 35, panelY + panelH - 100, 70, 0);
    }
    else if (currentPage == 2) { // TRANG 3: HỆ THỐNG CHI TIẾT
        DrawTextCentered(hdc, L"THÔNG SỐ TRẬN ĐẤU & CƠ CHẾ AI", panelY + 85, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Bold);
        
        int curY = panelY + 135;
        SetTextColor(hdc, Colour::GRAY_DARKEST);
        SelectObject(hdc, GlobalFont::Default);
        
        auto DrawSystemRow = [&](const std::wstring& title, const std::wstring& desc, COLORREF tCol) {
            DrawTextCentered(hdc, title, curY, screenWidth, tCol, GlobalFont::Bold);
            curY += 28;
            DrawTextCentered(hdc, desc, curY, screenWidth, Colour::GRAY_DARKEST, GlobalFont::Default);
            curY += 52;
        };

        DrawSystemRow(L"1. CẤP ĐỘ AI", L"Easy: Di chuyển ngẫu nhiên. Medium: Biết chặn đòn đơn giản. Hard: AI tìm kiếm nước đi tối ưu (Legendary).", Colour::CYAN_NORMAL);
        DrawSystemRow(L"2. THỜI GIAN ĐẾM NGƯỢC", L"Mỗi lượt có 15s/30s/45s. Nếu hết giờ mà chưa đi, máy sẽ tự động bỏ lượt hoặc đánh ngẫu nhiên.", Colour::ORANGE_NORMAL);
        DrawSystemRow(L"3. MỤC TIÊU CHIẾN THẮNG", L"Số trận thắng cần đạt để giành cúp. Ví dụ: Target = 3 thì phe nào thắng 3 trận trước sẽ vô địch.", Colour::GREEN_NORMAL);
        DrawSystemRow(L"4. LƯU & TẢI TRẬN", L"Dữ liệu được lưu dưới dạng file nhị phân, đảm bảo bạn có thể tiếp tục hành trình vô địch bất cứ lúc nào.", Colour::GRAY_DARK);
    }

    // 5. Page Indicators (Dots)
    int dotY = panelY + panelH + 30;
    int dotSpacing = 35;
    int startDotX = screenWidth / 2 - dotSpacing;
    for (int i = 0; i < 3; i++) {
        Gdiplus::SolidBrush dotBrush(i == currentPage ? GdipColour::TITLE_BORDER : GdipColour::SHADOW_MED);
        g.FillEllipse(&dotBrush, startDotX + i * dotSpacing - 6, dotY - 6, 12, 12);
        if (i == currentPage) {
            Gdiplus::Pen auraPen(Gdiplus::Color(100, 0, 100, 255), 2);
            g.DrawEllipse(&auraPen, startDotX + i * dotSpacing - 9, dotY - 9, 18, 18);
        }
    }

    // 6. Navigation Hints Arrows
    float arrowPulse = sin(g_GlobalAnimTime * 4.0f) * 8;
    int arrowY = panelY + panelH / 2 - 25;
    SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, Colour::BLUE_DARKEST);

    if (currentPage > 0) {
        RECT rLeft = { (int)(25 + arrowPulse), arrowY, (int)(105 + arrowPulse), arrowY + 50 };
        DrawTextW(hdc, L"<--", -1, &rLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    if (currentPage < 2) {
        RECT rRight = { (int)(screenWidth - 105 - arrowPulse), arrowY, (int)(screenWidth - 25 - arrowPulse), arrowY + 50 };
        DrawTextW(hdc, L"-->", -1, &rRight, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Nút thoát
    int pulse = (int)(sin(g_GlobalAnimTime * 5.0f) * 40 + 60);
    COLORREF escColor = RGB(pulse, 0, 0); 
    DrawTextCentered(hdc, L"Nhấn [ESC] để quay lại Menu chính", screenHeight - 50, screenWidth, escColor, GlobalFont::Note);
}
