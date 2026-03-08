#include "SettingScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../SystemModules/ConfigLoader.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/Localization.h"
#include <raylib.h>
#include <string>

const int TOTAL_SETTING_ITEMS = 7;

void UpdateSettingScreen(ScreenState& currentState, GameConfig* config, int& selectedOption) {
    // Di chuyển lên/xuống
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = TOTAL_SETTING_ITEMS - 1;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        selectedOption++;
        if (selectedOption >= TOTAL_SETTING_ITEMS) selectedOption = 0;
    }

    // Thay đổi giá trị bằng phím A/D hoặc Trái/Phải
    int direction = 0;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ENTER)) direction = 1;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) direction = -1;

    if (direction != 0) {
        switch (selectedOption) {
        case 0: // Bật/Tắt BGM
            config->isBgmEnabled = !config->isBgmEnabled;
            if (!config->isBgmEnabled) StopBGM();
            else PlayBGM("Asset/audio/bgm_menu.wav");
            break;
        case 1: // Âm lượng BGM
            config->bgmVolume += direction * 10;
            if (config->bgmVolume > 100) config->bgmVolume = 100;
            if (config->bgmVolume < 0) config->bgmVolume = 0;
            break;
        case 2: // Bật/Tắt SFX
            config->isSfxEnabled = !config->isSfxEnabled;
            break;
        case 3: // Âm lượng SFX
            config->sfxVolume += direction * 10;
            if (config->sfxVolume > 100) config->sfxVolume = 100;
            if (config->sfxVolume < 0) config->sfxVolume = 0;
            break;
        case 4: // Ngôn ngữ
            // Sử dụng APP_LANG_ đã sửa ở bước trước
            config->currentLang = (config->currentLang == APP_LANG_VIETNAMESE) ? APP_LANG_ENGLISH : APP_LANG_VIETNAMESE;
            LoadLanguageFile(config->currentLang);
            break;
        case 5: // Chủ đề bàn cờ
        {
            int themeVal = (int)config->currentTheme + direction;
            if (themeVal > (int)THEME_RETRO) themeVal = (int)THEME_CLASSIC;
            if (themeVal < (int)THEME_CLASSIC) themeVal = (int)THEME_RETRO;
            config->currentTheme = (BoardTheme)themeVal;
        }
        break;
        case 6: // Lưu và Thoát
            if (IsKeyPressed(KEY_ENTER)) {
                SaveConfig(config, "Asset/config.ini");
                currentState = SCREEN_MENU;
            }
            break;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        SaveConfig(config, "Asset/config.ini");
        currentState = SCREEN_MENU;
    }
}

void RenderSettingScreen(const GameConfig* config, int selectedOption, int screenWidth, int screenHeight) {
    int titleY = 60;
    DrawTextCentered(titleY, screenWidth, "--- CAI DAT (SETTINGS) ---", 40, DARKBLUE);

    int startY = 150;
    int spacing = 60;

    for (int i = 0; i < TOTAL_SETTING_ITEMS; i++) {
        std::string label = "";
        std::string value = "";
        Color itemColor = (i == selectedOption) ? GOLD : DARKGRAY;
        int fontSize = (i == selectedOption) ? 30 : 25;

        switch (i) {
        case 0: label = "Nhac nen (BGM):"; value = config->isBgmEnabled ? "BAT (ON)" : "TAT (OFF)"; break;
        case 1: label = "Am luong BGM:"; value = std::to_string(config->bgmVolume) + "%"; break;
        case 2: label = "Hieu ung (SFX):"; value = config->isSfxEnabled ? "BAT (ON)" : "TAT (OFF)"; break;
        case 3: label = "Am luong SFX:"; value = std::to_string(config->sfxVolume) + "%"; break;
        case 4: label = "Ngon ngu (Language):"; value = (config->currentLang == APP_LANG_VIETNAMESE) ? "Tieng Viet" : "English"; break;
        case 5: label = "Chu de (Theme):";
            value = (config->currentTheme == THEME_CLASSIC) ? "Classic" : (config->currentTheme == THEME_NEON ? "Neon" : "Retro");
            break;
        case 6: label = ">>> LUU VA QUAY LAI (SAVE & BACK) <<<"; value = ""; break;
        }

        int yPos = startY + i * spacing;

        if (i == 6) {
            DrawTextCentered(yPos + 20, screenWidth, label.c_str(), fontSize, itemColor);
        }
        else {
            // Vẽ nhãn bên trái
            DrawTextNormal(screenWidth / 4, yPos, label.c_str(), fontSize, itemColor);
            // Vẽ giá trị bên phải
            DrawTextNormal(screenWidth / 2 + 50, yPos, value.c_str(), fontSize, (i == selectedOption) ? RED : MAROON);

            // Nếu là âm lượng, vẽ thêm một thanh bar nhỏ cho đẹp
            if (i == 1 || i == 3) {
                int vol = (i == 1) ? config->bgmVolume : config->sfxVolume;
                DrawRectangle(screenWidth / 2 + 150, yPos + 5, 100, 20, LIGHTGRAY);
                DrawRectangle(screenWidth / 2 + 150, yPos + 5, vol, 20, BLUE);
            }
        }
    }

    DrawTextCentered(screenHeight - 40, screenWidth, "A/D hoac LEFT/RIGHT de thay doi gia tri", 20, GRAY);
}