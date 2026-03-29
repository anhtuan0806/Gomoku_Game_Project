#include "MenuScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"

const int TOTAL_MENU_ITEMS = 6;

// Chuyển sang chuỗi wide-character (L"") để hỗ trợ Unicode tiếng Việt
const wchar_t* menuItems[TOTAL_MENU_ITEMS] = {
    L"1. Bắt Đầu Chơi (Play Game)",
    L"2. Tải Game (Load Game)",
    L"3. Cài Đặt (Settings)",
    L"4. Hướng Dẫn (Guide)",
    L"5. Giới Thiệu (About)",
    L"6. Thoát (Exit)"
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
    // 0. Xóa nền màn hình cũ bằng màu xám nhạt
    RECT rect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::GRAY_LIGHTEST);
    FillRect(hdc, &rect, hBg);
    DeleteObject(hBg);

    // 1. Vẽ tiêu đề Game (Áp dụng GlobalFont::Title cho cỡ lớn)
    int titleY = screenHeight / 4 - 60;

    // Tham số HFONT cuối cùng bị bỏ trống -> Tự động dùng GlobalFont::Default
    DrawTextCentered(hdc, L"==================================", titleY, screenWidth, Colour::BLUE_LIGHT);

    // Truyền GlobalFont::Title để hiển thị phông to, đậm
    DrawTextCentered(hdc, L"CARO & TIC-TAC-TOE", titleY + 40, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

    DrawTextCentered(hdc, L"==================================", titleY + 100, screenWidth, Colour::BLUE_LIGHT);

    // 2. In danh sách các mục Menu
    int startY = screenHeight / 2 - 20;
    int spacing = 50;

    for (int i = 0; i < TOTAL_MENU_ITEMS; i++) {
        int currentY = startY + i * spacing;

        if (i == selectedOption) {
            // Mục đang chọn: Dùng GlobalFont::Bold và màu cam nổi bật
            std::wstring highlightedText = L">>  " + std::wstring(menuItems[i]) + L"  <<";
            DrawTextCentered(hdc, highlightedText, currentY, screenWidth, Colour::ORANGE_NORMAL, GlobalFont::Bold);
        }
        else {
            // Mục bình thường: Dùng GlobalFont::Default và màu xám tối
            DrawTextCentered(hdc, menuItems[i], currentY, screenWidth, Colour::GRAY_DARK);
        }
    }

    // 3. Vẽ hướng dẫn điều khiển
    DrawTextCentered(hdc, L"Dùng W/S/UP/DOWN để di chuyển, ENTER để chọn", screenHeight - 50, screenWidth, Colour::GRAY_NORMAL);
}