#include "GuildScreen.h"
#include "../SystemModules/Localization.h"
#include "../SystemModules/AudioSystem.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include <string>

void UpdateGuildScreen(ScreenState& currentState, int& currentPage, WPARAM wParam) {
    bool isRepeat = (wParam & 0x20000) != 0;
    WPARAM key = wParam & 0xFFFF;

    // Throttling: Giới hạn 80ms cho phím nhấn tay, 150ms cho phím giữ (Repeat)
    static DWORD lastMoveTime = 0;
    DWORD now = GetTickCount();
    bool canMove = (now - lastMoveTime > (DWORD)(isRepeat ? 150 : 80));

    if (key == VK_ESCAPE) {
        if (!canMove) return;
        if (!isRepeat) PlaySFX("sfx_move");
        currentState = SCREEN_MENU;
        currentPage = 0;
        lastMoveTime = now;
    }
    else if (key == VK_RIGHT || key == 'D' || key == 'd') {
        if (!canMove) return;
        if (currentPage < 2) {
            currentPage++;
            if (!isRepeat) PlaySFX("sfx_move");
        }
        lastMoveTime = now;
    }
    else if (key == VK_LEFT || key == 'A' || key == 'a') {
        if (!canMove) return;
        if (currentPage > 0) {
            currentPage--;
            if (!isRepeat) PlaySFX("sfx_move");
        }
        lastMoveTime = now;
    }
}

void RenderGuildScreen(HDC hdc, int screenWidth, int screenHeight, int currentPage)
{
    Gdiplus::Graphics g(hdc);

    DrawProceduralStadium(g, screenWidth, screenHeight);
    Gdiplus::SolidBrush whiteGlassBrush(Gdiplus::Color(60, 255, 255, 255));
    g.FillRectangle(&whiteGlassBrush, 0, 0, screenWidth, screenHeight);

    COLORREF bannerColors[] = { ToCOLORREF(Palette::OrangeNormal), ToCOLORREF(Palette::CyanNormal), ToCOLORREF(Palette::GreenNormal) };
    std::wstring titles[] = { GetText("guild_tab1"), GetText("guild_tab2"), GetText("guild_tab3") };
    DrawPixelBanner(g, hdc, titles[currentPage].c_str(), screenWidth / 2, UIScaler::SY(70), UIScaler::SX(500), ToCOLORREF(Palette::White), bannerColors[currentPage]);

    int panelW = screenWidth - UIScaler::SX(80);
    int panelH = UIScaler::SY(480);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = UIScaler::SY(135);

    int colW = panelW / 2;
    int colPadding = UIScaler::SX(35);

    Gdiplus::SolidBrush whitePanel(ToGdiColor(Theme::GlassWhite));
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);
    Gdiplus::Pen panelPen(ToGdiColor(Theme::PanelGoldBorder), 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    DrawPixelBanner(g, hdc, L"", screenWidth / 2, panelY + UIScaler::SY(40),
                    panelW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(255, 215, 0), "Asset/models/bg/badge.txt");
    Gdiplus::Pen dividerPen(Gdiplus::Color(50, 0, 0, 0), 1);

    if (currentPage == 0) {
        DrawTextCentered(hdc, GetText("guild_1_title"), panelY + UIScaler::SY(85), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);
        g.DrawLine(&dividerPen, (INT)(panelX + colW), (INT)(panelY + UIScaler::SY(120)), (INT)(panelX + colW), (INT)(panelY + panelH - UIScaler::SY(40)));

        int curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, GetText("guild_1_caro"), curY, screenWidth - colW, ToCOLORREF(Palette::OrangeNormal), GlobalFont::Bold);
        curY += UIScaler::SY(50);
        RECT rLeft = {panelX + colPadding, curY, panelX + colW - colPadding, panelY + panelH - UIScaler::SY(20)};
        SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
        SelectObject(hdc, GlobalFont::Default);
        std::wstring caroTxt = GetText("guild_1_caro_desc");
        DrawTextW(hdc, caroTxt.c_str(), -1, &rLeft, DT_LEFT | DT_WORDBREAK);

        curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, GetText("guild_1_ttt"), curY, screenWidth + colW, ToCOLORREF(Palette::CyanNormal), GlobalFont::Bold);
        curY += UIScaler::SY(50);
        RECT rRight = { panelX + colW + colPadding, curY, panelX + panelW - colPadding, panelY + panelH - UIScaler::SY(20) };
        std::wstring tttTxt = GetText("guild_1_ttt_desc");
        DrawTextW(hdc, tttTxt.c_str(), -1, &rRight, DT_LEFT | DT_WORDBREAK);
        DrawPixelFootball(g, screenWidth / 2, panelY + panelH - UIScaler::SY(60), UIScaler::S(50));
    }
    else if (currentPage == 1) {
        DrawTextCentered(hdc, GetText("guild_2_title"), panelY + UIScaler::SY(85), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);
        g.DrawLine(&dividerPen, (INT)(panelX + colW), (INT)(panelY + UIScaler::SY(120)), (INT)(panelX + colW), (INT)(panelY + panelH - UIScaler::SY(40)));

        int curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, GetText("guild_2_move"), curY, screenWidth - colW, ToCOLORREF(Palette::CyanNormal), GlobalFont::Bold);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, GetText("guild_2_move1"), curY, screenWidth - colW, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Default);
        curY += UIScaler::SY(35);
        DrawTextCentered(hdc, GetText("guild_2_move2"), curY, screenWidth - colW, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, GetText("guild_2_move3"), curY, screenWidth - colW, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Default);

        curY = panelY + UIScaler::SY(135);
        DrawTextCentered(hdc, GetText("guild_2_act"), curY, screenWidth + colW, ToCOLORREF(Palette::OrangeNormal), GlobalFont::Bold);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, GetText("guild_2_act1"), curY, screenWidth + colW, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Default);
        curY += UIScaler::SY(35);
        DrawTextCentered(hdc, GetText("guild_2_act2"), curY, screenWidth + colW, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
        curY += UIScaler::SY(60);
        DrawTextCentered(hdc, GetText("guild_2_act3"), curY, screenWidth + colW, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Default);
        curY += UIScaler::SY(50);
        DrawTextCentered(hdc, GetText("guild_2_act4"), curY, screenWidth + colW, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Default);
        DrawPixelAvatar(g, screenWidth / 2 - UIScaler::SX(35), panelY + panelH - UIScaler::SY(100), UIScaler::S(70), 0);
    }
    else if (currentPage == 2) {
        DrawTextCentered(hdc, GetText("guild_3_title"), panelY + UIScaler::SY(85), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Bold);
        int curY = panelY + UIScaler::SY(135);
        SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
        SelectObject(hdc, GlobalFont::Default);

        auto DrawSystemRow = [&](const std::wstring& title, const std::wstring& desc, COLORREF tCol) {
            DrawTextCentered(hdc, title, curY, screenWidth, tCol, GlobalFont::Bold);
            curY += UIScaler::SY(28);
            DrawTextCentered(hdc, desc, curY, screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Default);
            curY += UIScaler::SY(52);
        };
        DrawSystemRow(GetText("guild_3_ai"), GetText("guild_3_ai_desc"), ToCOLORREF(Palette::CyanNormal));
        DrawSystemRow(GetText("guild_3_time"), GetText("guild_3_time_desc"), ToCOLORREF(Palette::OrangeNormal));
        DrawSystemRow(GetText("guild_3_target"), GetText("guild_3_target_desc"), ToCOLORREF(Palette::GreenNormal));
        DrawSystemRow(GetText("guild_3_save"), GetText("guild_3_save_desc"), ToCOLORREF(Palette::GrayDark));
    }

    int dotY = panelY + panelH + UIScaler::SY(30);
    int dotSpacing = UIScaler::SX(35);
    int startDotX = screenWidth / 2 - dotSpacing;
    for (int i = 0; i < 3; i++)
    {
        Gdiplus::SolidBrush dotBrush(i == currentPage ? ToGdiColor(Theme::TitleBorder) : ToGdiColor(Theme::ShadowMed));
        int dotR = UIScaler::S(6);
        g.FillEllipse(&dotBrush, startDotX + i * dotSpacing - dotR, dotY - dotR, dotR * 2, dotR * 2);
        if (i == currentPage)
        {
            Gdiplus::Pen auraPen(Gdiplus::Color(100, 0, 100, 255), 2);
            int auraR = UIScaler::S(9);
            g.DrawEllipse(&auraPen, startDotX + i * dotSpacing - auraR, dotY - auraR, auraR * 2, auraR * 2);
        }
    }

    float arrowPulse = (float)(sin(g_GlobalAnimTime * 4.0f) * UIScaler::SX(8));
    int arrowY = panelY + panelH / 2 - UIScaler::SY(25);
    SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, ToCOLORREF(Palette::BlueDarkest));

    if (currentPage > 0)
    {
        RECT rLeft = {(int)(UIScaler::SX(25) + arrowPulse), arrowY, (int)(UIScaler::SX(105) + arrowPulse), arrowY + UIScaler::SY(50)};
        DrawTextW(hdc, L"<--", -1, &rLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    if (currentPage < 2)
    {
        RECT rRight = {(int)(screenWidth - UIScaler::SX(105) - arrowPulse), arrowY, (int)(screenWidth - UIScaler::SX(25) - arrowPulse), arrowY + UIScaler::SY(50)};
        DrawTextW(hdc, L"-->", -1, &rRight, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    int pulse = (int)(sin(g_GlobalAnimTime * 5.0f) * 40 + 60);
    COLORREF escColor = RGB(pulse, 0, 0);
    DrawTextCentered(hdc, GetText("about_esc"), screenHeight - UIScaler::SY(50), screenWidth, escColor, GlobalFont::Note);
}
