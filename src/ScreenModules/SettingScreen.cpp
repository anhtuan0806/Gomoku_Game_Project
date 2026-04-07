#include "SettingScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/ConfigLoader.h" 
#include "../ApplicationTypes/PlayState.h"
#include <string>

const int TOTAL_SETTING_ITEMS = 7;

void ProcessSettingInput(ScreenState& currentState, GameConfig* config, int selectedOption, int direction, bool isEnterPressed) {
    if (direction == 0 && !isEnterPressed) {
        return;
    }

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
            if (config->bgmVolume > 100) {
                config->bgmVolume = 100;
            }
            if (config->bgmVolume < 0) {
                config->bgmVolume = 0;
            }
        }
        break;

    case 2: // Hiệu ứng (SFX)
        if (isEnterPressed) {
            config->isSfxEnabled = !config->isSfxEnabled;
        }
        else {
            config->isSfxEnabled = (direction == 1);
        }
        break;

    case 3: // Âm lượng SFX
        if (direction != 0) {
            config->sfxVolume += direction * 10;
            if (config->sfxVolume > 100) {
                config->sfxVolume = 100;
            }
            if (config->sfxVolume < 0) {
                config->sfxVolume = 0;
            }
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

        if (themeVal > (int)THEME_RETRO) {
            themeVal = (int)THEME_CLASSIC;
        }
        if (themeVal < (int)THEME_CLASSIC) {
            themeVal = (int)THEME_RETRO;
        }
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
    if (keyCode == 0) {
        return;
    }

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
    if (keyCode == 'D' || keyCode == VK_RIGHT) {
        direction = 1;
    }
    if (keyCode == 'A' || keyCode == VK_LEFT) {
        direction = -1;
    }

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

    // 2. Bảng Form Kính Trắng — rộng và cao hơn để chứa đủ 7 dòng
    int panelW = 720;
    int panelH = 580;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - 10;

    Gdiplus::SolidBrush whitePanel(GdipColour::GLASS_WHITE);
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(GdipColour::PANEL_GREEN_BORDER, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề Pixel Banner
    DrawPixelBanner(g, hdc, L"THIẾT LẬP KỸ THUẬT", screenWidth / 2, panelY + 40,
        panelW - 20, Colour::WHITE, RGB(50, 220, 80));

    // 4. Layout 2 cột — Label bên trái, Value/Control bên phải
    int startY  = panelY + 105;
    int spacing = 52;                  // đủ cao để chữ không chạm nhau

    int col1X   = panelX + 16;
    int col1W   = 300;                 // cột nhãn
    int col2X   = panelX + 330;
    int col2W   = panelW - 330 - 16;  // cột giá trị — lấy hết phần còn lại

    SetBkMode(hdc, TRANSPARENT);

    for (int i = 0; i < TOTAL_SETTING_ITEMS; i++) {
        std::wstring label = L"";
        std::wstring value = L"";

        COLORREF labelColor = Colour::GRAY_DARKEST;
        COLORREF valColor   = Colour::GRAY_DARK;
        HFONT fontItem = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;

        // Màu giá trị pulse cam khi đang chọn
        if (i == selectedOption) {
            int rCol = (int)(180 + sin(g_GlobalAnimTime * 12.0f) * 75);
            valColor = RGB(255, max(0, min(255, 255 - rCol)), 0);
        }

        switch (i) {
        case 0: 
            label = L"Nhạc nền Sân:";  
            value = config->isBgmEnabled ? L"< BẬT (ON) >"  : L"< TẮT (OFF) >"; 
            break;
        case 1: 
            label = L"Âm lượng Nhạc:";           
            value = L""; 
            break;
        case 2: 
            label = L"Tạp âm Thi đấu (SFX):";   
            value = config->isSfxEnabled ? L"< BẬT (ON) >"  : L"< TẮT (OFF) >"; 
            break;
        case 3: 
            label = L"Âm lượng SFX:";            
            value = L""; 
            break;
        case 4: 
            label = L"Ngôn ngữ Bình luận:";      
            value = (config->currentLang == APP_LANG_VI) ? L"< Tiếng Việt >" : L"< English >"; 
            break;
        case 5: 
            label = L"Chủ đề Nền:";
            value = (config->currentTheme == THEME_CLASSIC) ? L"< Sân Cỏ Anh >"
                  : (config->currentTheme == THEME_NEON)    ? L"< Sân Neon >"
                  :                                           L"< Retro Matrix >"; 
            break;
        case 6: 
            label = L""; 
            value = L""; 
            break; // Nút Xác Nhận — vẽ riêng
        }

        int yPos = startY + i * spacing;

        if (i == 6) {
            // --- Nút Xác Nhận Ra Sân ---
            COLORREF btnColor = Colour::BLUE_DARKEST;
            if (i == selectedOption) {
                int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
                btnColor = RGB(max(0, min(255, 255 - gCol)), 100, 255);

                // Nền highlight nút khi chọn
                Gdiplus::SolidBrush btnBg(Gdiplus::Color(80, 0, 120, 255));
                g.FillRectangle(&btnBg, panelX + 60, yPos + 4, panelW - 120, spacing - 8);
            }
            RECT btnRect = { panelX, yPos, panelX + panelW, yPos + spacing };
            SetTextColor(hdc, btnColor);
            HFONT oldF = (HFONT)SelectObject(hdc, (i == selectedOption ? GlobalFont::Title : GlobalFont::Bold));
            DrawTextW(hdc, L"== [ QUAY LẠI CHỈ ĐẠO ] ==", -1, &btnRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(hdc, oldF);
        } 
        else {
            // --- Dòng nhãn (cột trái) ---
            DrawColTextSetting(hdc, label, col1X, yPos, col1W, labelColor, GlobalFont::Bold, DT_RIGHT);

            // --- Cột phải: thanh trượt hoặc giá trị text ---
            if (i == 1 || i == 3) {
                int vol  = (i == 1) ? config->bgmVolume : config->sfxVolume;
                int barX = col2X + 4;
                int barY = yPos + (spacing - 16) / 2;   // căn dọc giữa dòng
                int barW = 200;
                int barH = 16;

                // Nền thanh
                Gdiplus::SolidBrush bgBrush(GdipColour::BAR_TRACK);
                g.FillRectangle(&bgBrush, barX, barY, barW, barH);

                // Phần đã kéo
                Gdiplus::Color fillC = (i == selectedOption) ? GdipColour::BAR_FILL_SELECTED
                                                              : GdipColour::BAR_FILL_NORMAL;
                Gdiplus::SolidBrush fillBrush(fillC);
                g.FillRectangle(&fillBrush, barX, barY, vol * 2, barH);

                // Nút kéo (thumb)
                int thumbX = barX + vol * 2 - 5;
                Gdiplus::SolidBrush thumbBrush(GdipColour::WithAlpha(fillC, 255));
                g.FillRectangle(&thumbBrush, thumbX, barY - 2, 10, barH + 4);

                // % hiển thị bên phải thanh
                DrawColTextSetting(hdc, std::to_wstring(vol) + L"%",
                    barX + barW + 8, yPos, col2W - barW - 8, valColor, fontItem, DT_LEFT);
            } 
            else {
                DrawColTextSetting(hdc, value, col2X, yPos, col2W, valColor, fontItem, DT_LEFT);
            }
        }
    }

    // 5. Gợi ý phím — nằm dưới cùng màn hình
    DrawTextCentered(hdc, L"A / D: Thay đổi  |  W / S: Chọn mục  |  ESC / Enter: Lưu & Thoát",
        screenHeight - 48, screenWidth, Colour::WHITE, GlobalFont::Note);
}

