#include "SettingScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/ConfigLoader.h" 
#include "../ApplicationTypes/PlayState.h"
#include <string>

const int TOTAL_SETTING_ITEMS = 7;

void ProcessSettingInput(ScreenState& currentState, GameConfig* config, int selectedOption, int direction, bool isEnterPressed) {
    if (direction == 0 && !isEnterPressed) return;

    switch (selectedOption) {
    case 0: // Nhạc nền (BGM)
        if (isEnterPressed) 
            config->isBgmEnabled = !config->isBgmEnabled;
        else 
            config->isBgmEnabled = (direction == 1);

        if (!config->isBgmEnabled) StopBGM();
        else 
            //PlayBGM("Asset/audio/bgm_menu.wav");
        break;

    case 1: // Âm lượng BGM
        if (direction != 0) {
            config->bgmVolume += direction * 10;
            if (config->bgmVolume > 100) config->bgmVolume = 100;
            if (config->bgmVolume < 0) config->bgmVolume = 0;
        }
        break;

    case 2: // Hiệu ứng (SFX)
        if (isEnterPressed) config->isSfxEnabled = !config->isSfxEnabled;
        else config->isSfxEnabled = (direction == 1);
        break;

    case 3: // Âm lượng SFX
        if (direction != 0) {
            config->sfxVolume += direction * 10;
            if (config->sfxVolume > 100) config->sfxVolume = 100;
            if (config->sfxVolume < 0) config->sfxVolume = 0;
        }
        break;

    case 4: // Ngôn ngữ
        if (direction != 0 || isEnterPressed) {
            // Đã sửa lỗi: APP_LANG_VI thay vì APP_LANG_VIETNAMESE
            config->currentLang = (config->currentLang == APP_LANG_VI) ? APP_LANG_EN : APP_LANG_VI;
        }
        break;

    case 5: // Chủ đề
    {
        int step = (direction != 0) ? direction : 1;
        int themeVal = (int)config->currentTheme + step;

        if (themeVal > (int)THEME_RETRO) themeVal = (int)THEME_CLASSIC;
        if (themeVal < (int)THEME_CLASSIC) themeVal = (int)THEME_RETRO;
        config->currentTheme = (BoardTheme)themeVal;
        break;
    }

    case 6: // Lưu và Thoát
        if (isEnterPressed) {
            SaveConfig(config, "Asset/config.ini");
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

    if (keyCode == 'W' || keyCode == VK_UP) {
        selectedOption = (selectedOption - 1 < 0) ? TOTAL_SETTING_ITEMS - 1 : selectedOption - 1;
        return;
    }
    else if (keyCode == 'S' || keyCode == VK_DOWN) {
        selectedOption = (selectedOption + 1 >= TOTAL_SETTING_ITEMS) ? 0 : selectedOption + 1;
        return;
    }

    int direction = 0;
    if (keyCode == 'D' || keyCode == VK_RIGHT) direction = 1;
    if (keyCode == 'A' || keyCode == VK_LEFT) direction = -1;

    bool isEnterPressed = (keyCode == VK_RETURN);
    ProcessSettingInput(currentState, config, selectedOption, direction, isEnterPressed);
}

void RenderSettingScreen(HDC hdc, const GameConfig* config, int selectedOption, int screenWidth, int screenHeight) {
    // Phủ nền trước khi vẽ
    RECT bgRect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBgBrush = CreateSolidBrush(Colour::GRAY_LIGHTEST);
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);

    int titleY = 60;
    DrawTextCentered(hdc, L"--- CÀI ĐẶT (SETTINGS) ---", titleY, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

    int startY = 150;
    int spacing = 60;

    for (int i = 0; i < TOTAL_SETTING_ITEMS; i++) {
        std::wstring label = L"";
        std::wstring value = L"";
        COLORREF itemColor = (i == selectedOption) ? Colour::YELLOW_NORMAL : Colour::GRAY_DARK;
        HFONT itemFont = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;

        switch (i) {
        case 0: label = L"Nhạc nền (BGM):"; value = config->isBgmEnabled ? L"BẬT (ON)" : L"TẮT (OFF)"; break;
        case 1: label = L"Âm lượng BGM:"; value = std::to_wstring(config->bgmVolume) + L"%"; break;
        case 2: label = L"Hiệu ứng (SFX):"; value = config->isSfxEnabled ? L"BẬT (ON)" : L"TẮT (OFF)"; break;
        case 3: label = L"Âm lượng SFX:"; value = std::to_wstring(config->sfxVolume) + L"%"; break;
        case 4: label = L"Ngôn ngữ (Language):"; value = (config->currentLang == APP_LANG_VI) ? L"Tiếng Việt" : L"English"; break;
        case 5: label = L"Chủ đề (Theme):";
            value = (config->currentTheme == THEME_CLASSIC) ? L"Classic" : (config->currentTheme == THEME_NEON ? L"Neon" : L"Retro");
            break;
        case 6: label = L">>> LƯU VÀ QUAY LẠI (SAVE & BACK) <<<"; value = L""; break;
        }

        int yPos = startY + i * spacing;

        if (i == 6) {
            DrawTextCentered(hdc, label, yPos + 20, screenWidth, itemColor, itemFont);
        }
        else {
            SetTextColour(hdc, itemColor);
            HFONT hOldFont = (HFONT)SelectObject(hdc, itemFont);

            // Căn lề lùi vào một chút cho đẹp
            TextOutW(hdc, screenWidth / 6, yPos, label.c_str(), (int)label.length());

            SetTextColour(hdc, (i == selectedOption) ? Colour::RED_NORMAL : Colour::RED_DARKEST);
            TextOutW(hdc, screenWidth / 2, yPos, value.c_str(), (int)value.length());

            // Vẽ thanh Volume
            if (i == 1 || i == 3) {
                int vol = (i == 1) ? config->bgmVolume : config->sfxVolume;
                int barX = screenWidth / 2 + 120;

                RECT fullBar = { barX, yPos + 10, barX + 100, yPos + 25 };
                HBRUSH bBrush = CreateSolidBrush(Colour::GRAY_LIGHT);
                FillRect(hdc, &fullBar, bBrush);
                DeleteObject(bBrush);

                RECT activeBar = { barX, yPos + 10, barX + vol, yPos + 25 };
                HBRUSH aBrush = CreateSolidBrush(Colour::BLUE_NORMAL);
                FillRect(hdc, &activeBar, aBrush);
                DeleteObject(aBrush);
            }
            SelectObject(hdc, hOldFont);
        }
    }

    DrawTextCentered(hdc, L"A/D hoặc LEFT/RIGHT để thay đổi giá trị", screenHeight - 40, screenWidth, Colour::GRAY_NORMAL);
}