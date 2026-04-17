#include "SettingScreen.h"
#include "../SystemModules/Localization.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/ConfigLoader.h"
#include "../ApplicationTypes/PlayState.h"
#include <string>

const int TOTAL_SETTING_ITEMS = 6;

void ProcessSettingInput(ScreenState& currentState, GameConfig* config, int selectedOption, int direction, bool isEnterPressed, bool isRepeat) {
    if (direction == 0 && !isEnterPressed) return;

    switch (selectedOption) {
    case 0:
        if (isEnterPressed) config->isBgmEnabled = !config->isBgmEnabled;
        else config->isBgmEnabled = (direction == 1);

        if (!config->isBgmEnabled) StopBGM();
        else PlayBGM("Asset/audio/c1.mp3");
        if (!isRepeat) PlaySFX("sfx_move");
        break;
    case 1:
        if (direction != 0) {
            config->bgmVolume += direction * 10;
            if (config->bgmVolume > 100) config->bgmVolume = 100;
            if (config->bgmVolume < 0) config->bgmVolume = 0;
            UpdateBGMVolume();
            if (!isRepeat) PlaySFX("sfx_move");
        }
        break;
    case 2:
        if (isEnterPressed) config->isSfxEnabled = !config->isSfxEnabled;
        else config->isSfxEnabled = (direction == 1);
        if (!isRepeat) PlaySFX("sfx_move");
        break;
    case 3:
        if (direction != 0) {
            config->sfxVolume += direction * 10;
            if (config->sfxVolume > 100) config->sfxVolume = 100;
            if (config->sfxVolume < 0) config->sfxVolume = 0;
            if (!isRepeat) PlaySFX("sfx_move");
        }
        break;
    case 4:
        if (direction != 0 || isEnterPressed) {
            config->currentLang = (config->currentLang == APP_LANG_VI) ? APP_LANG_EN : APP_LANG_VI;
            LoadLanguageFile(config->currentLang);
            if (!isRepeat) PlaySFX("sfx_move");
        }
        break;
    case 5:
        if (isEnterPressed) {
            SaveConfig(config, "Asset/config.ini");
            PlaySFX("sfx_select");
            currentState = SCREEN_MENU;
        }
        break;
    }
}

void UpdateSettingScreen(ScreenState& currentState, GameConfig* config, int& selectedOption, WPARAM keyCode) {
    if (keyCode == 0) return;
    if (keyCode == VK_ESCAPE) {
        SaveConfig(config, "Asset/config.ini");
        currentState = SCREEN_MENU;
        return;
    }

    bool isRepeat = (keyCode & 0x20000) != 0;

    // Throttling: Giới hạn 80ms cho phím nhấn tay, 150ms cho phím giữ (Repeat)
    static DWORD lastMoveTime = 0;
    DWORD now = GetTickCount();
    bool canMove = (now - lastMoveTime > (DWORD)(isRepeat ? 150 : 80));

    if (keyCode == 'W' || keyCode == VK_UP) {
        if (!canMove) return;
        do {
            selectedOption = (selectedOption - 1 < 0) ? TOTAL_SETTING_ITEMS - 1 : selectedOption - 1;
        } while ((selectedOption == 1 && !config->isBgmEnabled) || (selectedOption == 3 && !config->isSfxEnabled));
        if (!isRepeat) PlaySFX("sfx_move");
        lastMoveTime = now;
        return;
    }
    else if (keyCode == 'S' || keyCode == VK_DOWN) {
        if (!canMove) return;
        do {
            selectedOption = (selectedOption + 1 >= TOTAL_SETTING_ITEMS) ? 0 : selectedOption + 1;
        } while ((selectedOption == 1 && !config->isBgmEnabled) || (selectedOption == 3 && !config->isSfxEnabled));
        if (!isRepeat) PlaySFX("sfx_move");
        lastMoveTime = now;
        return;
    }

    int direction = 0;
    if (keyCode == 'D' || keyCode == VK_RIGHT) {
        if (!canMove) return;
        direction = 1;
        lastMoveTime = now;
    }
    if (keyCode == 'A' || keyCode == VK_LEFT) {
        if (!canMove) return;
        direction = -1;
        lastMoveTime = now;
    }

    bool isEnterPressed = (keyCode == VK_RETURN);
    ProcessSettingInput(currentState, config, selectedOption, direction, isEnterPressed, isRepeat);
}

void DrawColTextSetting(HDC hdc, const std::wstring& text, int x, int y, int width, COLORREF color, HFONT font, UINT format) {
    SetTextColor(hdc, color);
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    RECT rect = { x, y, x + width, y + UIScaler::SY(50) };
    DrawTextW(hdc, text.c_str(), -1, &rect, format | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}

void RenderSettingScreen(HDC hdc, const GameConfig* config, int selectedOption, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    DrawProceduralStadium(g, screenWidth, screenHeight);

    int panelW = UIScaler::SX(720);
    int panelH = UIScaler::SY(580);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - UIScaler::SY(10);

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

    for (int i = 0; i < TOTAL_SETTING_ITEMS; i++) {
        std::wstring label = L"";
        std::wstring value = L"";
        COLORREF labelColor = ToCOLORREF(Palette::GrayDarkest);
        COLORREF valColor = ToCOLORREF(Palette::GrayDark);
        HFONT fontItem = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;
        bool isDisabled = (i == 1 && !config->isBgmEnabled) || (i == 3 && !config->isSfxEnabled);

        if (i == selectedOption) {
            int rCol = (int)(180 + sin(g_GlobalAnimTime * 12.0f) * 75);
            valColor = RGB(255, max(0, min(255, 255 - rCol)), 0);
        }

        if (isDisabled) {
            labelColor = RGB(150, 150, 150);
            valColor = RGB(150, 150, 150);
        }

        switch (i) {
        case 0: label = GetText("setting_bgm") + L":"; value = config->isBgmEnabled ? L" [ " + GetText("btn_on") + L" ]" : L" [ " + GetText("btn_off") + L" ]"; break;
        case 1: label = GetText("setting_bgm_vol") + L":"; value = L""; break;
        case 2: label = GetText("setting_sfx") + L":"; value = config->isSfxEnabled ? L" [ " + GetText("btn_on") + L" ]" : L" [ " + GetText("btn_off") + L" ]"; break;
        case 3: label = GetText("setting_sfx_vol") + L":"; value = L""; break;
        case 4: label = GetText("setting_lang") + L":"; value = (config->currentLang == APP_LANG_VI) ? L"< Tiếng Việt >" : L"< English >"; break;
        case 5: label = L""; value = L""; break;
        }

        int yPos = startY + i * spacing;

        if (i == 5) {
            COLORREF btnColor = ToCOLORREF(Palette::BlueDarkest);
            if (i == selectedOption) {
                int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
                btnColor = RGB(max(0, min(255, 255 - gCol)), 100, 255);
                Gdiplus::SolidBrush btnBg(Gdiplus::Color(80, 0, 120, 255));
                g.FillRectangle(&btnBg, panelX + UIScaler::SX(60), yPos + UIScaler::SY(4), panelW - UIScaler::SX(120), spacing - UIScaler::SY(8));
            }
            RECT btnRect = { panelX, yPos, panelX + panelW, yPos + spacing };
            SetTextColor(hdc, btnColor);
            HFONT oldF = (HFONT)SelectObject(hdc, (i == selectedOption ? GlobalFont::Title : GlobalFont::Bold));
            std::wstring btnTxt = L"== [ " + GetText("btn_back") + L" ] ==";
            DrawTextW(hdc, btnTxt.c_str(), -1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(hdc, oldF);
        }
        else {
            DrawColTextSetting(hdc, label, col1X, yPos, col1W, labelColor, GlobalFont::Bold, DT_RIGHT);
            if (i == 0 || i == 2) {
                bool enabled = (i == 0) ? config->isBgmEnabled : config->isSfxEnabled;
                COLORREF tColor = enabled ? RGB(0, 180, 50) : RGB(220, 50, 50);
                DrawColTextSetting(hdc, value, col2X, yPos, col2W, tColor, (i == selectedOption ? GlobalFont::Bold : GlobalFont::Default), DT_LEFT);
            }
            if (i == 1 || i == 3) {
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
            else if (i == 4) {
                DrawColTextSetting(hdc, value, col2X, yPos, col2W, valColor, fontItem, DT_LEFT);
            }
        }
    }
    DrawTextCentered(hdc, GetText("setting_hint"), screenHeight - UIScaler::SY(48), screenWidth, ToCOLORREF(Palette::White), GlobalFont::Note);
}
