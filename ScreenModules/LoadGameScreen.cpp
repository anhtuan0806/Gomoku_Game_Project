#include "LoadGameScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../SystemModules/SaveLoadSystem.h"
#include "../SystemModules/AudioSystem.h"
#include <raylib.h>
#include <string>

const int TOTAL_LOAD_ITEMS = 4;
// Danh sách các mục hiển thị trên màn hình
const char* loadItems[TOTAL_LOAD_ITEMS] = {
    "1. Ban luu 1 (Save Slot 1)",
    "2. Ban luu 2 (Save Slot 2)",
    "3. Ban luu 3 (Save Slot 3)",
    "4. Quay lai Menu (Back)"
};

void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::string& statusMessage) {
    // Di chuyển lên
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = TOTAL_LOAD_ITEMS - 1;
        statusMessage = ""; // Xóa thông báo lỗi cũ khi đổi mục khác
    }

    // Di chuyển xuống
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        selectedOption++;
        if (selectedOption >= TOTAL_LOAD_ITEMS) selectedOption = 0;
        statusMessage = "";
    }

    // Chọn mục
    if (IsKeyPressed(KEY_ENTER)) {
        if (selectedOption == 3) {
            // Chọn "Quay lại Menu"
            currentState = SCREEN_MENU;
            statusMessage = "";
        }
        else {
            // Tạo tên file tương ứng (VD: Asset/save_slot_1.bin)
            std::string filename = "Asset/save_slot_" + std::to_string(selectedOption + 1) + ".bin";

            // Gọi hệ thống LoadMatchData để nạp dữ liệu vào playState
            if (LoadMatchData(playState, filename.c_str())) {
                PlaySFX("Asset/audio/success.wav"); // Âm thanh thành công (nếu có)
                currentState = SCREEN_PLAY;         // Load thành công -> Chuyển thẳng vào ván đấu!
                statusMessage = "";
            }
            else {
                // Thất bại: Không tìm thấy file hoặc file hỏng
                statusMessage = "Loi: Khong tim thay hoac file luu bi hong!";
                PlaySFX("Asset/audio/error.wav"); // Âm thanh báo lỗi
            }
        }
    }

    // Bấm ESC để thoát nhanh về Menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentState = SCREEN_MENU;
        statusMessage = "";
    }
}

void RenderLoadGameScreen(int selectedOption, const std::string& statusMessage, int screenWidth, int screenHeight) {
    // 1. Vẽ tiêu đề màn hình
    int titleY = screenHeight / 4 - 60;
    DrawTextCentered(titleY, screenWidth, "==================================", 30, SKYBLUE);
    DrawTextCentered(titleY + 40, screenWidth, "TAI GAME (LOAD GAME)", 50, BLUE);
    DrawTextCentered(titleY + 100, screenWidth, "==================================", 30, SKYBLUE);

    // 2. Vẽ danh sách các khe lưu game (Save Slots)
    int startY = screenHeight / 2 - 20;
    int spacing = 50;

    for (int i = 0; i < TOTAL_LOAD_ITEMS; i++) {
        int currentY = startY + i * spacing;

        if (i == selectedOption) {
            // Mục đang được chọn: Thêm mũi tên, phóng to và tô màu Vàng
            std::string highlightedText = ">>  " + std::string(loadItems[i]) + "  <<";
            DrawTextCentered(currentY, screenWidth, highlightedText.c_str(), 35, GOLD);
        }
        else {
            // Mục bình thường
            DrawTextCentered(currentY, screenWidth, loadItems[i], 30, DARKGRAY);
        }
    }

    // 3. Hiển thị thông báo lỗi (nếu có) bằng màu Đỏ ở dưới cùng danh sách
    if (!statusMessage.empty()) {
        DrawTextCentered(startY + TOTAL_LOAD_ITEMS * spacing + 20, screenWidth, statusMessage.c_str(), 25, RED);
    }

    // 4. Hướng dẫn phím điều khiển
    DrawTextCentered(screenHeight - 50, screenWidth, "W/S: Di chuyen | ENTER: Chon | ESC: Quay lai", 20, GRAY);
}