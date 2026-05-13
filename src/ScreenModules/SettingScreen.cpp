#include "SettingScreen.h"
#include "../SystemModules/Localization.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../RenderAPI/PixelLayout.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/ConfigLoader.h"
#include "../SystemModules/EngineStats.h"
#include "../ApplicationTypes/PlayState.h"
#include <string>

/** @file SettingScreen.cpp
 *  @brief Màn cài đặt: xử lý thay đổi cấu hình (BGM/SFX, ngôn ngữ, VFX, FPS) và vẽ UI.
 */

const int TOTAL_SETTING_ITEMS = 8;

/** @brief Áp dụng thay đổi cho mục cài đặt tương ứng.
 *  @param currentState Tham chiếu trạng thái màn (có thể chuyển về MENU khi lưu/thoát).
 *  @param config Cấu hình game để cập nhật.
 *  @param selectedOption Mục cài đặt đang thao tác.
 *  @param direction Hướng (1/-1) khi thay đổi giá trị bằng phím trái/phải.
 *  @param isEnterPressed Flag nếu người dùng nhấn Enter.
 *  @param isRepeat Flag nếu phím đang ở trạng thái autorepeat.
 */
bool ProcessSettingInput(ScreenState &currentState, GameConfig *config, int selectedOption, int direction, bool isEnterPressed, bool isRepeat)
{
    if (direction == 0 && !isEnterPressed)
        return false;
        
    bool changed = true;

    switch (selectedOption)
    {
    case 0:
        if (isEnterPressed)
            config->isBgmEnabled = !config->isBgmEnabled;
        else
            config->isBgmEnabled = (direction == 1);

        if (!config->isBgmEnabled)
            stopBgm();
        else
            playBgm("Asset/audio/c1.mp3");
        if (!isRepeat)
            playSfx("sfx_move");
        break;
    case 1:
        if (direction != 0)
        {
            config->bgmVolume += direction * 10;
            if (config->bgmVolume > 100)
                config->bgmVolume = 100;
            if (config->bgmVolume < 0)
                config->bgmVolume = 0;
            updateBgmVolume();
            if (!isRepeat)
                playSfx("sfx_move");
        }
        break;
    case 2:
        if (isEnterPressed)
            config->isSfxEnabled = !config->isSfxEnabled;
        else
            config->isSfxEnabled = (direction == 1);
        if (!isRepeat)
            playSfx("sfx_move");
        break;
    case 3:
        if (direction != 0)
        {
            config->sfxVolume += direction * 10;
            if (config->sfxVolume > 100)
                config->sfxVolume = 100;
            if (config->sfxVolume < 0)
                config->sfxVolume = 0;
            if (!isRepeat)
                playSfx("sfx_move");
        }
        break;
    case 4:
        if (direction != 0 || isEnterPressed)
        {
            config->currentLang = (config->currentLang == APP_LANG_VI) ? APP_LANG_EN : APP_LANG_VI;
            LoadLanguageFile(config->currentLang);
            if (!isRepeat)
                playSfx("sfx_move");
        }
        break;
    case 5:
        if (isEnterPressed || direction != 0)
        {
            config->isVisualEffectsEnabled = !config->isVisualEffectsEnabled;
            if (!isRepeat)
                playSfx("sfx_move");
        }
        break;
    case 6:
        if (direction != 0 || isEnterPressed)
        {
            // Toggle giữa 30 và 60 FPS, áp dụng ngay lập tức
            config->fpsLimit = (config->fpsLimit == 60) ? 30 : 60;
            EngineStats::SetTargetFPS(static_cast<double>(config->fpsLimit));
            if (!isRepeat)
                playSfx("sfx_move");
        }
        break;
    case 7:
        if (isEnterPressed)
        {
            SaveConfig(config, "Asset/config.ini");
            playSfx("sfx_select");
            currentState = SCREEN_MENU;
        }
        break;
    }
    return changed;
}

/** @brief Xử lý input cho màn Settings (di chuyển lựa chọn, gọi ProcessSettingInput).
 *  @param currentState Tham chiếu trạng thái màn hình.
 *  @param config Cấu hình game.
 *  @param selectedOption Tham chiếu mục đang chọn.
 *  @param keyCode Mã phím/flags (WM_KEY/WM_CHAR encoded).
 */
bool UpdateSettingScreen(ScreenState &currentState, GameConfig *config, int &selectedOption, WPARAM keyCode)
{
    if (keyCode == 0)
        return false;
    if (keyCode == VK_ESCAPE)
    {
        SaveConfig(config, "Asset/config.ini");
        currentState = SCREEN_MENU;
        return true;
    }

    bool isRepeat = (keyCode & 0x20000) != 0;

    // Throttling: Giới hạn 80ms cho phím nhấn tay, 150ms cho phím giữ (Repeat)
    static ULONGLONG lastMoveTime = 0;
    ULONGLONG now = GetTickCount64();
    bool canMove = (now - lastMoveTime > (ULONGLONG)(isRepeat ? 150 : 80));

    if (keyCode == 'W' || keyCode == VK_UP)
    {
        if (!canMove)
            return false;
        do
        {
            selectedOption = (selectedOption - 1 < 0) ? TOTAL_SETTING_ITEMS - 1 : selectedOption - 1;
        } while ((selectedOption == 1 && !config->isBgmEnabled) || (selectedOption == 3 && !config->isSfxEnabled));
        if (!isRepeat)
            playSfx("sfx_move");
        lastMoveTime = now;
        return true;
    }
    else if (keyCode == 'S' || keyCode == VK_DOWN)
    {
        if (!canMove)
            return false;
        do
        {
            selectedOption = (selectedOption + 1 >= TOTAL_SETTING_ITEMS) ? 0 : selectedOption + 1;
        } while ((selectedOption == 1 && !config->isBgmEnabled) || (selectedOption == 3 && !config->isSfxEnabled));
        if (!isRepeat)
            playSfx("sfx_move");
        lastMoveTime = now;
        return true;
    }

    int direction = 0;
    if (keyCode == 'D' || keyCode == VK_RIGHT)
    {
        if (!canMove)
            return false;
        direction = 1;
        lastMoveTime = now;
    }
    if (keyCode == 'A' || keyCode == VK_LEFT)
    {
        if (!canMove)
            return false;
        direction = -1;
        lastMoveTime = now;
    }

    bool isEnterPressed = (keyCode == VK_RETURN);
    return ProcessSettingInput(currentState, config, selectedOption, direction, isEnterPressed, isRepeat);
}

/** @brief Vẽ ô chữ cho màn Setting (hàm tiện ích giống DrawColText ở các màn khác).
 *  @param hdc Device context.
 *  @param text Nội dung cần vẽ.
 *  @param x, y, width Vùng ô.
 *  @param color Màu chữ.
 *  @param font Font sử dụng.
 *  @param format Cờ DrawText.
 */
void DrawColTextSetting(HDC hdc, const std::wstring &text, int x, int y, int width, COLORREF color, HFONT font, UINT format)
{
    SetTextColor(hdc, color);
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    RECT rect = {x, y, x + width, y + UIScaler::SY(50)};
    DrawTextW(hdc, text.c_str(), -1, &rect, format | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}

/** @brief Vẽ giao diện màn Setting, hiển thị các mục có thể thay đổi.
 *  @param hdc Device context.
 *  @param config Cấu hình hiện tại để hiển thị giá trị.
 *  @param selectedOption Mục đang chọn (để highlight).
 *  @param screenWidth, screenHeight Kích thước vùng vẽ.
 */
void RenderSettingScreen(HDC hdc, const GameConfig *config, int selectedOption, int screenWidth, int screenHeight)
{
    Gdiplus::Graphics g(hdc);
    PixelLayout::ApplyPixelArtBlit(g);
    DrawProceduralStadium(g, screenWidth, screenHeight);

    int panelW = UIScaler::SX(720);
    int panelH = UIScaler::SY(680);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - UIScaler::SY(10);
    PixelLayout::AlignRectToPixelGrid(panelX, panelY, panelW, panelH);

    Gdiplus::SolidBrush whitePanel(ToGdiColor(Theme::GlassWhite));
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);
    Gdiplus::Pen panelPen(Gdiplus::Color(180, 50, 200, 80), 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    DrawPixelBanner(g, hdc, GetText("setting_title").c_str(), screenWidth / 2, panelY + UIScaler::SY(40),
                    panelW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(50, 220, 80), "Asset/models/bg/gears.txt");

    int startY = panelY + UIScaler::SY(105);
    int spacing = UIScaler::SY(52);
    int col1X = panelX + UIScaler::SX(16);
    int col1W = UIScaler::SX(300);
    int col2X = panelX + UIScaler::SX(330);
    int col2W = panelW - UIScaler::SX(330) - UIScaler::SX(16);

    SetBkMode(hdc, TRANSPARENT);

    for (int i = 0; i < TOTAL_SETTING_ITEMS; i++)
    {
        std::wstring label = L"";
        std::wstring value = L"";
        COLORREF labelColor = ToCOLORREF(Palette::GrayDarkest);
        COLORREF valColor = ToCOLORREF(Palette::GrayDark);
        HFONT fontItem = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;
        bool isDisabled = (i == 1 && !config->isBgmEnabled) || (i == 3 && !config->isSfxEnabled);

        if (i == selectedOption)
        {
            int rCol = (int)(180 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 12.f) * 75);
            valColor = RGB(255, max(0, min(255, 255 - rCol)), 0);
        }

        if (isDisabled)
        {
            labelColor = RGB(150, 150, 150);
            valColor = RGB(150, 150, 150);
        }

        switch (i)
        {
        case 0:
            label = GetText("setting_bgm") + L":";
            value = config->isBgmEnabled ? L" [ " + GetText("btn_on") + L" ]" : L" [ " + GetText("btn_off") + L" ]";
            break;
        case 1:
            label = GetText("setting_bgm_vol") + L":";
            value = L"";
            break;
        case 2:
            label = GetText("setting_sfx") + L":";
            value = config->isSfxEnabled ? L" [ " + GetText("btn_on") + L" ]" : L" [ " + GetText("btn_off") + L" ]";
            break;
        case 3:
            label = GetText("setting_sfx_vol") + L":";
            value = L"";
            break;
        case 4:
            label = GetText("setting_lang") + L":";
            value = (config->currentLang == APP_LANG_VI) ? L"< " + GetText("lang_vi") + L" >" : L"< " + GetText("lang_en") + L" >";
            break;
        case 5:
            label = GetText("setting_vfx") + L":";
            value = config->isVisualEffectsEnabled ? L" [ " + GetText("btn_on") + L" ]" : L" [ " + GetText("btn_off") + L" ]";
            break;
        case 6:
            label = GetText("setting_fps") + L":";
            value = L"< " + std::to_wstring(config->fpsLimit) + L" FPS >";
            break;
        case 7:
            label = L"";
            value = L"";
            break;
        }

        int yPos = startY + i * spacing;

        if (i == 7)
        {
            COLORREF btnColor = ToCOLORREF(Palette::BlueDarkest);
            if (i == selectedOption)
            {
                int gCol = (int)(150 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 15.f) * 105);
                btnColor = RGB(max(0, min(255, 255 - gCol)), 100, 255);
                Gdiplus::SolidBrush btnBg(Gdiplus::Color(80, 0, 120, 255));
                g.FillRectangle(&btnBg, panelX + UIScaler::SX(60), yPos + UIScaler::SY(4), panelW - UIScaler::SX(120), spacing - UIScaler::SY(8));
            }
            RECT btnRect = {panelX, yPos, panelX + panelW, yPos + spacing};
            SetTextColor(hdc, btnColor);
            HFONT oldF = (HFONT)SelectObject(hdc, (i == selectedOption ? GlobalFont::Title : GlobalFont::Bold));
            std::wstring btnTxt = L"== [ " + GetText("btn_back") + L" ] ==";
            DrawTextW(hdc, btnTxt.c_str(), -1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(hdc, oldF);
        }
        else
        {
            DrawColTextSetting(hdc, label, col1X, yPos, col1W, labelColor, GlobalFont::Bold, DT_RIGHT);
            if (i == 0 || i == 2 || i == 5)
            {
                bool enabled = (i == 0) ? config->isBgmEnabled : (i == 2 ? config->isSfxEnabled : config->isVisualEffectsEnabled);
                COLORREF tColor = enabled ? RGB(0, 180, 50) : RGB(220, 50, 50);
                DrawColTextSetting(hdc, value, col2X, yPos, col2W, tColor, (i == selectedOption ? GlobalFont::Bold : GlobalFont::Default), DT_LEFT);
            }
            if (i == 1 || i == 3)
            {
                int vol = (i == 1) ? config->bgmVolume : config->sfxVolume;
                int barX = col2X + UIScaler::SX(4);
                int barY = yPos + (spacing - UIScaler::SY(16)) / 2;
                int barW = UIScaler::SX(220);
                int barH = UIScaler::SY(14);
                Gdiplus::SolidBrush bgBrush(ToGdiColor(Theme::BarTrack));
                g.FillRectangle(&bgBrush, barX, barY, barW, barH);
                float percent = vol / 100.0f;
                Gdiplus::Color fillC = isDisabled ? Gdiplus::Color(100, 150, 150, 150) : ((i == selectedOption) ? ToGdiColor(Theme::BarFillSelected) : ToGdiColor(Theme::BarFillNormal));
                Gdiplus::SolidBrush fillBrush(fillC);
                g.FillRectangle(&fillBrush, barX, barY, (int)(barW * percent), barH);
                int thumbX = barX + (int)(barW * percent) - UIScaler::SX(5);
                Gdiplus::Color tC = isDisabled ? Gdiplus::Color(255, 100, 100, 100) : Gdiplus::Color(255, 230, 230, 230);
                Gdiplus::SolidBrush thumbBrush(tC);
                g.FillRectangle(&thumbBrush, thumbX, barY - UIScaler::SY(2), UIScaler::SX(10), barH + UIScaler::SY(4));
                DrawColTextSetting(hdc, std::to_wstring(vol) + L"%", barX + barW + UIScaler::SX(15), yPos, col2W - barW - UIScaler::SX(15), valColor, fontItem, DT_LEFT);
            }
            else if (i == 4 || i == 6)
            {
                DrawColTextSetting(hdc, value, col2X, yPos, col2W, valColor, fontItem, DT_LEFT);
            }
        }
    }
    DrawTextCentered(hdc, GetText("setting_hint"), screenHeight - UIScaler::SY(48), screenWidth, ToCOLORREF(Palette::White), GlobalFont::Note);
}
