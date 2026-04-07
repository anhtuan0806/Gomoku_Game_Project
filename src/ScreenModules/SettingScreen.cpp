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

// Hàm phụ trợ vẽ Text cân theo Cột  
void DrawColTextSetting(HDC hdc, const std::wstring& text, int x, int y, int width, COLORREF color, HFONT font, UINT format) {
    SetTextColor(hdc, color);
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    RECT rect = { x, y, x + width, y + 50 };
    DrawTextW(hdc, text.c_str(), -1, &rect, format | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}

void RenderSettingScreen(HDC hdc, const GameConfig* config, int selectedOption, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    
    // 1. Nền Sân Cổ Động Procedural
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // 2. Bảng Form Kính Trắng (White Glassmorphism)
    Gdiplus::SolidBrush whitePanel(GdipColour::GLASS_WHITE);
    int panelW = 650;
    int panelH = 500;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - 20;
    
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);
    
    Gdiplus::Pen panelPen(GdipColour::PANEL_GREEN_BORDER, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // Tiêu đề
    DrawTextCentered(hdc, L"--- THIẾT LẬP KỸ THUẬT ---", panelY + 30, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

    int startY = panelY + 100;
    int spacing = 45;
    
    int col1X = panelX + 20;
    int col1W = 280;
    int col2X = panelX + 320;
    int col2W = 300;

    for (int i = 0; i < TOTAL_SETTING_ITEMS; i++) {
        std::wstring label = L"";
        std::wstring value = L"";
        
        COLORREF labelColor = Colour::GRAY_DARKEST;
        COLORREF valColor = Colour::GRAY_DARK;
        HFONT fontItem = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;

        if (i == selectedOption) {
            int rCol = (int)(180 + sin(g_GlobalAnimTime * 12.0f) * 75);
            valColor = RGB(255, max(0, min(255, 255 - rCol)), 0); // Pulse Cam/Đỏ
        }

        switch (i) {
        case 0: label = L"Nhạc nền Sân vận động:"; value = config->isBgmEnabled ? L"< BẬT (ON) >" : L"< TẮT (OFF) >"; break;
        case 1: label = L"Âm lượng Nhạc:"; value = L""; break; // Đặc thù vẽ thanh Bar
        case 2: label = L"Tạp âm Thi đấu (SFX):"; value = config->isSfxEnabled ? L"< BẬT (ON) >" : L"< TẮT (OFF) >"; break;
        case 3: label = L"Âm lượng SFX:"; value = L""; break; // Đặc thù vẽ thanh Bar
        case 4: label = L"Ngôn ngữ Bình luận:"; value = (config->currentLang == APP_LANG_VI) ? L"< Tiếng Việt >" : L"< English >"; break;
        case 5: label = L"Chủ đề Nền (Theme):"; value = (config->currentTheme == THEME_CLASSIC) ? L"< Sân Cỏ Anh >" : (config->currentTheme == THEME_NEON ? L"< Sân Neon >" : L"< Retro Matrix >"); break;
        case 6: label = L"XÁC NHẬN RA SÂN"; value = L""; break;
        }

        int yPos = startY + i * spacing;

        if (i == 6) {
            COLORREF btnColor = Colour::BLUE_DARKEST;
            if (i == selectedOption) {
                int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
                btnColor = RGB(max(0, min(255, 255 - gCol)), 100, 255);
            }
            DrawTextCentered(hdc, L"== [ QUAY LẠI CHỈ ĐẠO ] ==", yPos + 30, screenWidth, btnColor, (i == selectedOption ? GlobalFont::Title : GlobalFont::Bold));
        }
        else {
            DrawColTextSetting(hdc, label, col1X, yPos, col1W, labelColor, GlobalFont::Bold, DT_RIGHT);
            
            if (i == 1 || i == 3) {
                // Vẽ Thanh Trượt Kỹ Thuật Số (Volume Bar)
                int vol = (i == 1) ? config->bgmVolume : config->sfxVolume;
                int barX = col2X + 20;
                int barY = yPos + 18;
                
                Gdiplus::SolidBrush bgBrush(GdipColour::BAR_TRACK);
                g.FillRectangle(&bgBrush, barX, barY, 200, 15);
                
                Gdiplus::Color fillC = (i == selectedOption) ? GdipColour::BAR_FILL_SELECTED : GdipColour::BAR_FILL_NORMAL;
                Gdiplus::SolidBrush fillBrush(fillC);
                g.FillRectangle(&fillBrush, barX, barY, vol * 2, 15); // x2 vì độ dài tổng là 200
                
                DrawColTextSetting(hdc, std::to_wstring(vol) + L"%", barX + 210, yPos, 80, valColor, fontItem, DT_LEFT);
            } else {
                DrawColTextSetting(hdc, value, col2X, yPos, col2W, valColor, fontItem, DT_LEFT);
            }
        }
    }

    DrawTextCentered(hdc, L"A/D: Thay đổi thông số Điều hành  |  ESC/Enter: Lưu Hồ sơ", screenHeight - 60, screenWidth, Colour::WHITE, GlobalFont::Note);
}