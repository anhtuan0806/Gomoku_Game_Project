#include "AboutScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../RenderAPI/PixelLayout.h"
#include "../SystemModules/Localization.h"
#include "../SystemModules/AudioSystem.h"
#include <string>

/** @file AboutScreen.cpp
 *  @brief Màn About: hiển thị thông tin nhóm, phiên bản và công nghệ sử dụng.
 */

/** @brief Xử lý input trên màn About (thoát về menu khi nhấn Esc).
 *  @param currentState Tham chiếu trạng thái màn hình (có thể đổi về `SCREEN_MENU`).
 *  @param wParam Mã phím/flags nhận từ main loop.
 */
bool UpdateAboutScreen(ScreenState &currentState, WPARAM wParam)
{
    bool isRepeat = (wParam & 0x20000) != 0;
    WPARAM key = wParam & 0xFFFF;

    // Throttling: Giới hạn 80ms cho phím nhấn tay
    static ULONGLONG lastMoveTime = 0;
    ULONGLONG now = GetTickCount64();
    bool canMove = (now - lastMoveTime > 80ULL);

    if (key == VK_ESCAPE)
    {
        if (!canMove)
            return false;
        if (!isRepeat)
            playSfx("sfx_move");
        currentState = SCREEN_MENU;
        lastMoveTime = now;
        return true;
    }
    return false;
}

/** @brief Vẽ nội dung màn About: banner, thông tin nhóm và công nghệ.
 *  @param hdc Device context để vẽ.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 */
void RenderAboutScreen(HDC hdc, int screenWidth, int screenHeight)
{
    Gdiplus::Graphics g(hdc);
    PixelLayout::ApplyPixelArtBlit(g);

    // 1. Nền sân vận động
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Lớp kính trắng mờ (Light Mode style)
    Gdiplus::SolidBrush whiteGlassBrush(Gdiplus::Color(60, 255, 255, 255));
    g.FillRectangle(&whiteGlassBrush, 0, 0, screenWidth, screenHeight);

    // 2. Banner Tiêu đề
    DrawPixelBanner(g, hdc, GetText("about_title").c_str(), screenWidth / 2, UIScaler::SY(80), UIScaler::SX(500), ToCOLORREF(Palette::White), ToCOLORREF(Palette::CyanNormal));

    // 3. Logo & Tên Game (Giữ nguyên vì là tên riêng)
    int trophySize = UIScaler::S(60);
    DrawPixelTrophy(g, screenWidth / 2, UIScaler::SY(165), trophySize);

    DrawTextCentered(hdc, L"CARO CHAMPIONS LEAGUE", UIScaler::SY(205), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Title);
    DrawTextCentered(hdc, L"Version 1.0 (Extreme Edition)", UIScaler::SY(250), screenWidth, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);

    // 4. Thông tin phát triển (Light Glassmorphism Panel)
    int panelW = UIScaler::SX(700);
    int panelH = UIScaler::SY(420);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = UIScaler::SY(280);

    Gdiplus::SolidBrush whitePanel(ToGdiColor(Theme::GlassWhite));
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(ToGdiColor(Theme::PanelOrangeBorder), 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề Pixel Banner (Chỉ giữ lại viền và icon làm vật trang trí)
    DrawPixelBanner(g, hdc, L"", screenWidth / 2, panelY + UIScaler::SY(40),
                    panelW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(255, 120, 0), "Asset/models/bg/history_pitch.txt");

    // 4. Nội dung About
    SetBkMode(hdc, TRANSPARENT);
    DrawTextCentered(hdc, GetText("about_team").c_str(), panelY + UIScaler::SY(80), screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold);

    int rowH = UIScaler::SY(35);
    int tableW = UIScaler::SX(600);
    int tableH = UIScaler::SY(175);
    int tableX = (screenWidth - tableW) / 2;
    int tableY = panelY + UIScaler::SY(115);
    int col1W = UIScaler::SX(380);

    // Header Background (Màu xanh nhạt cho tiêu đề bảng)
    Gdiplus::SolidBrush headerBg(Gdiplus::Color(60, 0, 150, 255));
    g.FillRectangle(&headerBg, tableX, tableY, tableW, rowH);

    // Vẽ Grid Lines
    Gdiplus::Pen gridPen(Gdiplus::Color(100, 100, 100, 100), 1.0f);
    for (int i = 0; i <= 5; i++)
    {
        g.DrawLine(&gridPen, (INT)tableX, (INT)(tableY + i * rowH), (INT)(tableX + tableW), (INT)(tableY + i * rowH));
    }
    g.DrawLine(&gridPen, (INT)tableX, (INT)tableY, (INT)tableX, (INT)(tableY + tableH));
    g.DrawLine(&gridPen, (INT)(tableX + col1W), (INT)tableY, (INT)(tableX + col1W), (INT)(tableY + tableH));
    g.DrawLine(&gridPen, (INT)(tableX + tableW), (INT)tableY, (INT)(tableX + tableW), (INT)(tableY + tableH));

    // Header Text
    HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, ToCOLORREF(Palette::White));
    RECT rHeader1 = {tableX, tableY, tableX + col1W, tableY + rowH};
    RECT rHeader2 = {tableX + col1W, tableY, tableX + tableW, tableY + rowH};
    DrawTextW(hdc, GetText("about_member").c_str(), -1, &rHeader1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hdc, GetText("about_id").c_str(), -1, &rHeader2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Body Text
    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
    struct Member
    {
        std::wstring name;
        std::wstring ms;
    };
    Member list[] = {
        {L"Trương Tuấn Anh", L"24120260"},
        {L"Lê Văn Quốc", L"24120421"},
        {L"Đặng Quang Sang", L"24120428"},
        {L"Trương Đoàn Công Thành", L"24120451"}};

    for (int i = 0; i < 4; i++)
    {
        RECT rName = {tableX + UIScaler::SX(20), tableY + (i + 1) * rowH, tableX + col1W - UIScaler::SX(5), tableY + (i + 2) * rowH};
        RECT rMs = {tableX + col1W, tableY + (i + 1) * rowH, tableX + tableW, tableY + (i + 2) * rowH};
        DrawTextW(hdc, list[i].name.c_str(), -1, &rName, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        DrawTextW(hdc, list[i].ms.c_str(), -1, &rMs, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(hdc, oldF);

    // Giảng viên hướng dẫn & Công nghệ
    int lineY = tableY + tableH + UIScaler::SY(20);
    DrawTextCentered(hdc, GetText("about_teacher").c_str(), lineY, screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);

    lineY += UIScaler::SY(35);
    DrawTextCentered(hdc, GetText("about_tech").c_str(), lineY, screenWidth, ToCOLORREF(Palette::GrayDark), GlobalFont::Bold);
    lineY += UIScaler::SY(28);
    DrawTextCentered(hdc, L"C++ / Win32 API / GDI+ (Raster & Procedural)", lineY, screenWidth, ToCOLORREF(Palette::GrayDark), GlobalFont::Default);

    // 5. Nút thoát
    int pulse = (int)(sin(g_GlobalAnimTime * 4.0f) * 30 + 100);
    COLORREF escColor = RGB(pulse, 0, 0); // Nhấp nháy Đỏ thẫm trên nền sáng
    DrawTextCentered(hdc, GetText("about_esc").c_str(), screenHeight - UIScaler::SY(60), screenWidth, escColor, GlobalFont::Note);
}