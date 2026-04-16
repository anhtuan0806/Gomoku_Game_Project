#include "GuildScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
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
    COLORREF bannerColors[] = { ToCOLORREF(Palette::OrangeNormal), ToCOLORREF(Palette::CyanNormal), ToCOLORREF(Palette::GreenNormal) };
    std::wstring titles[] = { L"CHẾ ĐỘ CHƠI", L"CÁCH ĐIỀU KHIỂN", L"CƠ CHẾ TRẬN ĐẤU" };
    DrawPixelBanner(g, hdc, titles[currentPage], screenWidth / 2, UIScaler::SY(70), UIScaler::SX(500), ToCOLORREF(Palette::White), bannerColors[currentPage]);

    // 3. Khung Panel nội dung (Light Mode)
    int panelW = screenWidth - UIScaler::SX(80);
    int panelH = UIScaler::SY(480);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = UIScaler::SY(135);
    
    // Các thông số chung cho cột (Cần thiết cho logic trang 1 & 2)
    int colW = panelW / 2;
    int colPadding = UIScaler::SX(35);

    Gdiplus::SolidBrush whitePanel(ToGdiColor(Theme::GlassWhite));
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(ToGdiColor(Theme::PanelGoldBorder), 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề Pixel Banner (Chỉ giữ lại viền và icon làm vật trang trí)
    DrawPixelBanner(g, hdc, L"", screenWidth / 2, panelY + UIScaler::SY(40),
        panelW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(255, 215, 0), "Asset/models/bg/badge.txt");
    Gdiplus::Pen dividerPen(Gdiplus::Color(50, 0, 0, 0), 1); // Vạch tối mờ trên nền sáng

    if (currentPage == 0) { // TRANG 1: HAI CHẾ ĐỘ CHƠI
        DrawTextCentered(hdc, L"HÃY CHỌN KỊCH BẢN CHIẾN THUẬT CỦA BẠN", panelY + UIScaler::SY(85), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);
        
        // Vẽ vạch ngăn đôi cột
        g.DrawLine(&dividerPen, (INT)(panelX + colW), (INT)(panelY + UIScaler::SY(120)), (INT)(panelX + colW), (INT)(panelY + panelH - UIScaler::SY(40)));

        // -- CỘT TRÁI: CARO (GOMOKU) --
        int curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, L"CARO - TRUYỀN THỐNG", curY, screenWidth - colW, ToCOLORREF(Palette::OrangeNormal), GlobalFont::Bold);
        curY += UIScaler::SY(50);
        RECT rLeft = { panelX + colPadding, curY, panelX + colW - colPadding, panelY + panelH - UIScaler::SY(20) };
        SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
        SelectObject(hdc, GlobalFont::Default);
        std::wstring caroTxt = L"- Kích thước sân: 15x15 ô cực rộng.\n"
                               L"- Luật thắng: Đạt đủ 5 quân cờ liên tiếp theo hàng ngang, dọc hoặc chéo.\n"
                               L"- Cơ chế: Đây là cuộc đấu trí tầm xa, bạn cần tính toán đa bước để bẫy đối thủ.\n"
                               L"- Ghi bàn: Mỗi lần đạt chuỗi 5, bạn sẽ ghi 1 bàn thắng vào lưới đối phương.";
        DrawTextW(hdc, caroTxt.c_str(), -1, &rLeft, DT_LEFT | DT_WORDBREAK);

        // -- CỘT PHẢI: TIC-TAC-TOE --
        curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, L"TIC-TAC-TOE - TỐC ĐỘ", curY, screenWidth + colW, ToCOLORREF(Palette::CyanNormal), GlobalFont::Bold);
        curY += UIScaler::SY(50);
        RECT rRight = { panelX + colW + colPadding, curY, panelX + panelW - colPadding, panelY + panelH - UIScaler::SY(20) };
        std::wstring tttTxt = L"- Kích thước sân: 3x3 ô nhỏ gọn.\n"
                              L"- Luật thắng: Chỉ cần 3 quân cờ liên tiếp.\n"
                              L"- Cơ chế: Trận đấu diễn ra cực nhanh, đòi hỏi phản xạ và chiếm lĩnh trung tâm ngay lập tức.\n"
                              L"- Thích hợp cho những trận đấu chớp nhoáng, phân định thắng bại tức thì.";
        DrawTextW(hdc, tttTxt.c_str(), -1, &rRight, DT_LEFT | DT_WORDBREAK);

        DrawPixelFootball(g, screenWidth / 2, panelY + panelH - UIScaler::SY(60), UIScaler::S(50));
    }
    else if (currentPage == 1) { // TRANG 2: ĐIỀU KHIỂN & THAO TÁC
        DrawTextCentered(hdc, L"THAO TÁC LINH HOẠT ĐỂ LÀM CHỦ TRẬN ĐẤU", panelY + UIScaler::SY(85), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);
        g.DrawLine(&dividerPen, (INT)(panelX + colW), (INT)(panelY + UIScaler::SY(120)), (INT)(panelX + colW), (INT)(panelY + panelH - UIScaler::SY(40)));

        // -- CỘT TRÁI: DI CHUYỂN --
        int curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, L"DI CHUYỂN TRÊN SÂN", curY, screenWidth - colW, ToCOLORREF(Palette::CyanNormal), GlobalFont::Bold);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, L"[Mũi tên] hoặc [WASD]", curY, screenWidth - colW, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Default);
        curY += UIScaler::SY(35);
        DrawTextCentered(hdc, L"Di chuyển con trỏ chọn ô.", curY, screenWidth - colW, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, L"Nhấn giữ để di chuyển nhanh", curY, screenWidth - colW, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Default);

        // -- CỘT PHẢI: HÀNH ĐỘNG --
        curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, L"HÀNH ĐỘNG QUYẾT ĐỊNH", curY, screenWidth + colW, ToCOLORREF(Palette::OrangeNormal), GlobalFont::Bold);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, L"[ENTER] hoặc [SPACE]", curY, screenWidth + colW, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Default);
        curY += UIScaler::SY(35);
        DrawTextCentered(hdc, L"Xác nhận đặt quân cờ.", curY, screenWidth + colW, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, L"[ESC]: Tạm dừng / Menu", curY, screenWidth + colW, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Default);
        curY += UIScaler::SY(50);
        DrawTextCentered(hdc, L"[S]: Lưu trận đấu nhanh", curY, screenWidth + colW, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Default);

        DrawPixelAvatar(g, screenWidth / 2 - UIScaler::SX(35), panelY + panelH - UIScaler::SY(100), UIScaler::S(70), 0);
    }
    else if (currentPage == 2) { // TRANG 3: HỆ THỐNG CHI TIẾT
        DrawTextCentered(hdc, L"THÔNG SỐ TRẬN ĐẤU & CƠ CHẾ AI", panelY + UIScaler::SY(85), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);
        
        int curY = panelY + UIScaler::SY(135);
        SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
        SelectObject(hdc, GlobalFont::Default);
        
        auto DrawSystemRow = [&](const std::wstring& title, const std::wstring& desc, COLORREF tCol) {
            DrawTextCentered(hdc, title, curY, screenWidth, tCol, GlobalFont::Bold);
            curY += UIScaler::SY(28);
            DrawTextCentered(hdc, desc, curY, screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Default);
            curY += UIScaler::SY(52);
        };

        DrawSystemRow(L"1. CẤP ĐỘ AI", L"Easy: Di chuyển ngẫu nhiên. Medium: Biết chặn đòn đơn giản. Hard: AI tìm kiếm nước đi tối ưu (Legendary).", ToCOLORREF(Palette::CyanNormal));
        DrawSystemRow(L"2. THỜI GIAN ĐẾM NGƯỢC", L"Mỗi lượt có 15s/30s/45s. Nếu hết giờ mà chưa đi, máy sẽ tự động bỏ lượt hoặc đánh ngẫu nhiên.", ToCOLORREF(Palette::OrangeNormal));
        DrawSystemRow(L"3. MỤC TIÊU CHIẾN THẮNG", L"Số bàn thắng cần ghi để thắng trận. Ví dụ: Target = 3 thì phe nào ghi 3 bàn trước sẽ vô địch trận BO đó.", ToCOLORREF(Palette::GreenNormal));
        DrawSystemRow(L"4. LƯU & TẢI TRẬN", L"Dữ liệu được lưu dưới dạng file nhị phân, đảm bảo bạn có thể tiếp tục hành trình vô địch bất cứ lúc nào.", ToCOLORREF(Palette::GrayDark));
    }

    // 5. Page Indicators (Dots)
    int dotY = panelY + panelH + UIScaler::SY(30);
    int dotSpacing = UIScaler::SX(35);
    int startDotX = screenWidth / 2 - dotSpacing;
    for (int i = 0; i < 3; i++) {
        Gdiplus::SolidBrush dotBrush(i == currentPage ? ToGdiColor(Theme::TitleBorder) : ToGdiColor(Theme::ShadowMed));
        int dotR = UIScaler::S(6);
        g.FillEllipse(&dotBrush, startDotX + i * dotSpacing - dotR, dotY - dotR, dotR * 2, dotR * 2);
        if (i == currentPage) {
            Gdiplus::Pen auraPen(Gdiplus::Color(100, 0, 100, 255), 2);
            int auraR = UIScaler::S(9);
            g.DrawEllipse(&auraPen, startDotX + i * dotSpacing - auraR, dotY - auraR, auraR * 2, auraR * 2);
        }
    }

    // 6. Navigation Hints Arrows
    float arrowPulse = sin(g_GlobalAnimTime * 4.0f) * UIScaler::SX(8);
    int arrowY = panelY + panelH / 2 - UIScaler::SY(25);
    SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, ToCOLORREF(Palette::BlueDarkest));

    if (currentPage > 0) {
        RECT rLeft = { (int)(UIScaler::SX(25) + arrowPulse), arrowY, (int)(UIScaler::SX(105) + arrowPulse), arrowY + UIScaler::SY(50) };
        DrawTextW(hdc, L"<--", -1, &rLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    if (currentPage < 2) {
        RECT rRight = { (int)(screenWidth - UIScaler::SX(105) - arrowPulse), arrowY, (int)(screenWidth - UIScaler::SX(25) - arrowPulse), arrowY + UIScaler::SY(50) };
        DrawTextW(hdc, L"-->", -1, &rRight, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Nút thoát
    int pulse = (int)(sin(g_GlobalAnimTime * 5.0f) * 40 + 60);
    COLORREF escColor = RGB(pulse, 0, 0); 
    DrawTextCentered(hdc, L"Nhấn [ESC] để quay lại Menu chính", screenHeight - UIScaler::SY(50), screenWidth, escColor, GlobalFont::Note);
}
