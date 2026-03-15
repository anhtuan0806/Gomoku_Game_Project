#include "SettingScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/Localization.h"
#include <string>

const int TOTAL_SETTING_ITEMS = 7;

void ProcessSettingInput(ScreenState& currentState, GameConfig* config, int selectedOption, int direction, bool isEnterPressed) {
    if (direction == 0 && !isEnterPressed) return;

    switch (selectedOption) {
    case 0: // Nhạc nền (BGM)
        if (isEnterPressed) config->isBgmEnabled = !config->isBgmEnabled;
        else config->isBgmEnabled = (direction == 1);

        if (!config->isBgmEnabled) StopBGM();
        else PlayBGM("Asset/audio/bgm_menu.wav");
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
            config->currentLang = (config->currentLang == APP_LANG_VIETNAMESE) ? APP_LANG_ENGLISH : APP_LANG_VIETNAMESE;
            LoadLanguageFile(config->currentLang);
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
    if (keyCode == 0) return; // Bỏ qua nếu không có phím nhấn

    // 1. Xử lý thoát nhanh (Phím ESC)
    if (keyCode == VK_ESCAPE) {
        SaveConfig(config, "Asset/config.ini");
        currentState = SCREEN_MENU;
        return;
    }

    // 2. Xử lý điều hướng Lên/Xuống
    if (keyCode == 'W' || keyCode == VK_UP) {
        selectedOption = (selectedOption - 1 < 0) ? TOTAL_SETTING_ITEMS - 1 : selectedOption - 1;
        return; // Đã chuyển mục thì không xử lý thay đổi giá trị
    }
    else if (keyCode == 'S' || keyCode == VK_DOWN) {
        selectedOption = (selectedOption + 1 >= TOTAL_SETTING_ITEMS) ? 0 : selectedOption + 1;
        return;
    }

    // 3. Phân tích tham số cho hàm Process
    int direction = 0;
    if (keyCode == 'D' || keyCode == VK_RIGHT) direction = 1;
    if (keyCode == 'A' || keyCode == VK_LEFT) direction = -1;

    bool isEnterPressed = (keyCode == VK_RETURN);

    // 4. Gọi hàm xử lý logic cài đặt
    ProcessSettingInput(currentState, config, selectedOption, direction, isEnterPressed);
}

void RenderSettingScreen(HDC hdc, const GameConfig* config, int selectedOption, int screenWidth, int screenHeight) {
    int titleY = 60;
    // GDI yêu cầu chuỗi Unicode (L"") cho các hàm API có hậu tố W
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
        case 4: label = L"Ngôn ngữ (Language):"; value = (config->currentLang == APP_LANG_VIETNAMESE) ? L"Tiếng Việt" : L"English"; break;
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
            // Thiết lập Font và màu sắc qua GDI
            SetTextColour(hdc, itemColor);
            HFONT hOldFont = (HFONT)SelectObject(hdc, itemFont);

            // Vẽ nhãn bên trái
            TextOutW(hdc, screenWidth / 4, yPos, label.c_str(), (int)label.length());

            // Vẽ giá trị bên phải
            SetTextColour(hdc, (i == selectedOption) ? Colour::RED_NORMAL : Colour::RED_DARKEST);
            TextOutW(hdc, screenWidth / 2 + 50, yPos, value.c_str(), (int)value.length());

            // Vẽ thanh âm lượng bằng GDI API (FillRect)
            if (i == 1 || i == 3) {
                int vol = (i == 1) ? config->bgmVolume : config->sfxVolume;

                // Vẽ nền thanh âm lượng (Màu xám)
                RECT bgRect = { screenWidth / 2 + 150, yPos + 10, screenWidth / 2 + 150 + 100, yPos + 25 };
                HBRUSH bgBrush = CreateSolidBrush(Colour::GRAY_LIGHT);
                FillRect(hdc, &bgRect, bgBrush);
                DeleteObject(bgBrush);

                // Vẽ mức âm lượng hiện tại (Màu xanh)
                RECT valRect = { screenWidth / 2 + 150, yPos + 10, screenWidth / 2 + 150 + vol, yPos + 25 };
                HBRUSH valBrush = CreateSolidBrush(Colour::BLUE_NORMAL);
                FillRect(hdc, &valRect, valBrush);
                DeleteObject(valBrush);
            }

            // Phục hồi tài nguyên Font
            SelectObject(hdc, hOldFont);
        }
    }

    DrawTextCentered(hdc, L"A/D hoặc LEFT/RIGHT để thay đổi giá trị", screenHeight - 40, screenWidth, Colour::GRAY_NORMAL, GlobalFont::Default);
}