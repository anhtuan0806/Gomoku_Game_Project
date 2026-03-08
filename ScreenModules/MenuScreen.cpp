#include "MenuScreen.h"
#include "../RenderAPI/UIComponents.h"
#include <raylib.h>
#include <string>

const int TOTAL_MENU_ITEMS = 6;
// Danh sách các mục trong Menu
const char* menuItems[TOTAL_MENU_ITEMS] = {
    "1. Bat Dau Choi (Play Game)",
    "2. Tai Game (Load Game)",
    "3. Cai Dat (Settings)",
    "4. Huong Dan (Guide)",
    "5. Gioi Thieu (About)",
    "6. Thoat (Exit)"
};

void UpdateMenuScreen(ScreenState& currentState, int& selectedOption) {
    // Raylib hỗ trợ nhận diện phím rất dễ dàng. 
    // Ở đây ta cho phép người dùng xài cả W/S hoặc Phím mũi tên Lên/Xuống

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        selectedOption--;
        if (selectedOption < 0) {
            selectedOption = TOTAL_MENU_ITEMS - 1; // Cuộn vòng xuống đáy
        }
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        selectedOption++;
        if (selectedOption >= TOTAL_MENU_ITEMS) {
            selectedOption = 0; // Cuộn vòng lên đầu
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        // Cập nhật trạng thái game dựa trên mục đang chọn
        switch (selectedOption) {
        case 0: currentState = SCREEN_PLAY; break;
        case 1: currentState = SCREEN_LOAD_GAME; break;
        case 2: currentState = SCREEN_SETTING; break;
        case 3: currentState = SCREEN_GUIDE; break;
        case 4: currentState = SCREEN_ABOUT; break;
        case 5: currentState = SCREEN_EXIT; break;
        }
    }
}

void RenderMenuScreen(int selectedOption, int screenWidth, int screenHeight) {
    // 1. Vẽ tiêu đề Game nổi bật ở 1/4 phía trên màn hình
    int titleY = screenHeight / 4 - 60;

    // Sử dụng màu của Raylib (SKYBLUE, BLUE...)
    DrawTextCentered(titleY, screenWidth, "==================================", 30, SKYBLUE);
    DrawTextCentered(titleY + 40, screenWidth, "CARO & TIC-TAC-TOE", 50, BLUE);
    DrawTextCentered(titleY + 100, screenWidth, "==================================", 30, SKYBLUE);

    // 2. In ra danh sách các mục Menu ở giữa màn hình
    int startY = screenHeight / 2 - 20; // Điểm bắt đầu vẽ menu
    int spacing = 50; // Khoảng cách giữa các dòng là 50 pixel

    for (int i = 0; i < TOTAL_MENU_ITEMS; i++) {
        int currentY = startY + i * spacing;

        if (i == selectedOption) {
            // Mục đang được chọn: Thêm mũi tên, phóng to cỡ chữ lên 35 và tô màu Vàng (GOLD)
            std::string highlightedText = ">>  " + std::string(menuItems[i]) + "  <<";
            DrawTextCentered(currentY, screenWidth, highlightedText.c_str(), 35, GOLD);
        }
        else {
            // Mục bình thường: Cỡ chữ 30, màu Xám đậm (DARKGRAY)
            DrawTextCentered(currentY, screenWidth, menuItems[i], 30, DARKGRAY);
        }
    }

    // 3. Vẽ hướng dẫn điều khiển ở sát đáy màn hình
    DrawTextCentered(screenHeight - 50, screenWidth, "Dung W/S de di chuyen, ENTER de chon", 20, GRAY);
}