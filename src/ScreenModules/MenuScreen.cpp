#include "MenuScreen.h"
#include "../SystemModules/AudioSystem.h"
#include "../RenderAPI/UIComponents.h"
#include "../SystemModules/Localization.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include <cmath>
#include <map>

const int TOTAL_MENU_ITEMS = 6;


void UpdateMenuScreen(ScreenState& currentState, int& selectedOption, WPARAM wParam) {
    // Bỏ qua nếu không có sự kiện phím hợp lệ
    if (wParam == 0) {
        return;
    }

    // Ủy quyền xử lý logic phím nhấn. 
    // Trong tương lai, nếu cần thêm hiệu ứng âm thanh khi chuyển mục menu (chẳng hạn PlaySFX("hover.wav")), 
    // thầy có thể bắt giá trị trả về (bool) của hàm này để kích hoạt.
    ProcessMenuInput(wParam, currentState, selectedOption);
}

bool ProcessMenuInput(WPARAM wParam, ScreenState& currentState, int& selectedOption) {
    bool hasChanged = false;

    // Loại bỏ hàm IsKeyPressed của Raylib (Polling), sử dụng trực tiếp wParam của Event
    if (wParam == 'W' || wParam == 'w' || wParam == VK_UP) {
        selectedOption--;
        if (selectedOption < 0) {
            selectedOption = TOTAL_MENU_ITEMS - 1;
        }
        PlaySFX("Asset/audio/move.wav", "sfx_move");
        hasChanged = true;
    }
    else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN) {
        selectedOption++;
        if (selectedOption >= TOTAL_MENU_ITEMS) {
            selectedOption = 0;
        }
        PlaySFX("Asset/audio/move.wav", "sfx_move");
        hasChanged = true;
    }
    else if (wParam == VK_RETURN || wParam == VK_SPACE) {
        PlaySFX("Asset/audio/select.wav", "sfx_select");
        switch (selectedOption) {
        case 0: 
            currentState = SCREEN_PLAY; 
            break;
        case 1: 
            currentState = SCREEN_LOAD_GAME; 
            break;
        case 2: 
            currentState = SCREEN_SETTING; 
            break;
        case 3: 
            currentState = SCREEN_GUIDE; 
            break;
        case 4: 
            currentState = SCREEN_ABOUT; 
            break;
        case 5: 
            currentState = SCREEN_EXIT; 
            break;
        }
        hasChanged = true;
    }

    return hasChanged; // Trạm điều phối (WndProc) sẽ kiểm tra cờ này để gọi InvalidateRect()
}

void RenderMenuScreen(HDC hdc, int selectedOption, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    
    // 0. Nền sân vận động (showFlashes=true: chỉ Menu mới có hiệu ứng tia chớp khán đài)
    DrawProceduralStadium(g, screenWidth, screenHeight, true);

    // Lớp kính trắng mờ để tạo phong cách Light Mode chuyên nghiệp
    Gdiplus::SolidBrush lightGlassBrush(Gdiplus::Color(80, 255, 255, 255));
    g.FillRectangle(&lightGlassBrush, 0, 0, screenWidth, screenHeight);

    // 1. Ve cup Vo Dich Pixel Art va Tieu de Game
    // ... (giữ nguyên logic tâng cúp)
    int cupYOffset = UIScaler::SY((int)(sin(g_GlobalAnimTime * 2.5f) * 10.0f));
    DrawPixelTrophy(g, screenWidth / 2, screenHeight / 4 - UIScaler::SY(85) + cupYOffset, UIScaler::S(100));

    // 1.5 Tải và Vẽ khối Logo Điểm Ảnh khổng lồ bằng hệ thống Load TXT Modular
    static PixelModel titleModel;
    static std::map<int, Gdiplus::Color> titlePalette;
    if (!titleModel.isLoaded) {
        titleModel = LoadPixelModel("Asset/models/bg/title_caro.txt");
        titlePalette[1] = ToGdiColor(Theme::TitleBorder);
        titlePalette[2] = ToGdiColor(Theme::TitleFill);
        titlePalette[3] = ToGdiColor(Theme::TitleShadow);
    }

    int titleYOffset = UIScaler::SY((int)(sin(g_GlobalAnimTime * 2.0f) * 6.0f));
    
    DrawPixelModel(g, titleModel, screenWidth / 2, screenHeight / 4 + UIScaler::SY(20) + titleYOffset, UIScaler::S(500), titlePalette);
    
    DrawTextCentered(hdc, L"CHAMPIONS LEAGUE", screenHeight / 4 + UIScaler::SY(75) + titleYOffset, screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Title);

    // 2. In danh sách các mục Menu
    int startY = screenHeight / 2 + UIScaler::SY(10);
    int spacing = UIScaler::SY(55);

    std::wstring menuItems[TOTAL_MENU_ITEMS] = {
        GetText("menu_play"),
        GetText("menu_load"),
        GetText("menu_settings"),
        GetText("menu_guild"),
        GetText("menu_about"),
        GetText("menu_exit")
    };

    for (int i = 0; i < TOTAL_MENU_ITEMS; i++) {
        int currentY = startY + i * spacing;

        if (i == selectedOption) {
            std::wstring highlightedText = std::wstring(menuItems[i]);
            
            // Hiệu ứng màu nhấp nháy cho mục đang chọn (Xanh lam -> Cyan)
            int gCol = (int)(180 + sin(g_GlobalAnimTime * 8.0f) * 75);
            COLORREF dynColor = RGB(0, max(0, min(255, gCol)), 255); 

            int wStrOffset = UIScaler::S((int)highlightedText.length() * 18 + 70); 
            DrawPixelFootball(g, screenWidth / 2 - wStrOffset, currentY + UIScaler::SY(38), UIScaler::S(48));
            DrawPixelFootball(g, screenWidth / 2 + wStrOffset, currentY + UIScaler::SY(38), UIScaler::S(48));

            DrawTextCentered(hdc, highlightedText, currentY, screenWidth, dynColor, GlobalFont::Title);
        }
        else {
            DrawTextCentered(hdc, menuItems[i], currentY + UIScaler::SY(6), screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold);
        }
    }

    // 3. Vẽ hướng dẫn điều khiển (Màu tối hơn cho nền sáng)
    DrawTextCentered(hdc, GetText("menu_instruct"), screenHeight - UIScaler::SY(50), screenWidth, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
}