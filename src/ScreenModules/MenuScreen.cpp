#include "MenuScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include <cmath>
#include <map>

const int TOTAL_MENU_ITEMS = 6;

// Chuyển sang chuỗi wide-character (L"") để hỗ trợ Unicode tiếng Việt
const wchar_t* menuItems[TOTAL_MENU_ITEMS] = {
    L"Bắt Đầu",
    L"Tải Băng",
    L"Cài Đặt Sân",
    L"Hướng Dẫn",
    L"Giới Thiệu",
    L"Thoát"
};

void UpdateMenuScreen(ScreenState& currentState, int& selectedOption, WPARAM wParam) {
    // Bỏ qua nếu không có sự kiện phím hợp lệ
    if (wParam == 0) 
        return;

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
        if (selectedOption < 0) selectedOption = TOTAL_MENU_ITEMS - 1;
        hasChanged = true;
    }
    else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN) {
        selectedOption++;
        if (selectedOption >= TOTAL_MENU_ITEMS) selectedOption = 0;
        hasChanged = true;
    }
    else if (wParam == VK_RETURN || wParam == VK_SPACE) {
        switch (selectedOption) {
        case 0: currentState = SCREEN_PLAY; break;
        case 1: currentState = SCREEN_LOAD_GAME; break;
        case 2: currentState = SCREEN_SETTING; break;
        case 3: currentState = SCREEN_GUIDE; break;
        case 4: currentState = SCREEN_ABOUT; break;
        case 5: currentState = SCREEN_EXIT; break;
        }
        hasChanged = true;
    }

    return hasChanged; // Trạm điều phối (WndProc) sẽ kiểm tra cờ này để gọi InvalidateRect()
}

void RenderMenuScreen(HDC hdc, int selectedOption, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    
    // 0. Nền sân vận động
    // Vẽ Sân vận động theo cấu trúc Ma Trận Thuật Toán (Procedural)
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Lớp kính phản quang mờ để nổi bật chữ
    Gdiplus::SolidBrush shadowBrush(GdipColour::SHADOW_PANEL);
    g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

    // 1. Ve cup Vo Dich Pixel Art va Tieu de Game
    // Ứng dụng Animation tâng Cúp
    int cupYOffset = (int)(sin(g_GlobalAnimTime * 2.5f) * 10.0f);
    DrawPixelTrophy(g, screenWidth / 2, screenHeight / 4 - 85 + cupYOffset, 100);

    // 1.5 Tải và Vẽ khối Logo Điểm Ảnh khổng lồ bằng hệ thống Load TXT Modular
    static PixelModel titleModel;
    static std::map<int, Gdiplus::Color> titlePalette;
    if (!titleModel.isLoaded) {
        titleModel = LoadPixelModel("Asset/models/title_caro.txt");
        titlePalette[1] = GdipColour::TITLE_BORDER;
        titlePalette[2] = GdipColour::TITLE_FILL;
        titlePalette[3] = GdipColour::TITLE_SHADOW;
    }

    int titleYOffset = (int)(sin(g_GlobalAnimTime * 2.0f) * 6.0f);
    
    // In Ma trận "CARO" (Pixel size = 12)
    DrawPixelModel(g, titleModel, screenWidth / 2, screenHeight / 4 + 20 + titleYOffset, 12, titlePalette);
    
    // Hậu tố "CHAMPIONS LEAGUE" bằng chữ VT323
    DrawTextCentered(hdc, L"CHAMPIONS LEAGUE", screenHeight / 4 + 75 + titleYOffset, screenWidth, Colour::YELLOW_NORMAL, GlobalFont::Title);

    // 2. In danh sách các mục Menu
    int startY = screenHeight / 2 + 10;
    int spacing = 55;

    for (int i = 0; i < TOTAL_MENU_ITEMS; i++) {
        int currentY = startY + i * spacing;

        if (i == selectedOption) {
            std::wstring highlightedText = std::wstring(menuItems[i]);
            
            // Hiệu ứng màu nhấp nháy chuyển từ Cam nhạt sang Vàng chói cho cảm giác năng động
            int gCol = (int)(150 + sin(g_GlobalAnimTime * 8.0f) * 105);
            COLORREF dynColor = RGB(255, max(0, min(255, gCol)), 0);

            // Vẽ Trái bóng 2 bên mục đang chọn (Tăng Size và hạ Offset Y để vừa với VT323)
            int wStrOffset = (int)highlightedText.length() * 18 + 70; 
            DrawPixelFootball(g, screenWidth / 2 - wStrOffset, currentY + 38, 48);
            DrawPixelFootball(g, screenWidth / 2 + wStrOffset, currentY + 38, 48);

            DrawTextCentered(hdc, highlightedText, currentY, screenWidth, dynColor, GlobalFont::Title);
        }
        else {
            DrawTextCentered(hdc, menuItems[i], currentY + 6, screenWidth, Colour::WHITE, GlobalFont::Bold);
        }
    }

    // 3. Vẽ hướng dẫn điều khiển
    DrawTextCentered(hdc, L"Dùng W/S/UP/DOWN để rê bóng, ENTER để sút (chọn)", screenHeight - 50, screenWidth, Colour::GRAY_LIGHT, GlobalFont::Note);
}