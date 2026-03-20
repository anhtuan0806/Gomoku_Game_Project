#include "LoadGameScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"

const int TOTAL_LOAD_ITEMS = 4;

// Sử dụng chuẩn chuỗi rộng (Wide string) cho Unicode
const wchar_t* loadItems[TOTAL_LOAD_ITEMS] = {
    L"1. Bản lưu 1 (Save Slot 1)",
    L"2. Bản lưu 2 (Save Slot 2)",
    L"3. Bản lưu 3 (Save Slot 3)",
    L"4. Quay lại Menu (Back)"
};

bool ProcessLoadGameInput(WPARAM wParam, ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage) {
    bool hasChanged = false;

    // Di chuyển lên
    if (wParam == 'W' || wParam == 'w' || wParam == VK_UP) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = TOTAL_LOAD_ITEMS - 1;
        statusMessage = L""; // Xóa thông báo lỗi cũ
        hasChanged = true;
    }
    // Di chuyển xuống
    else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN) {
        selectedOption++;
        if (selectedOption >= TOTAL_LOAD_ITEMS) selectedOption = 0;
        statusMessage = L"";
        hasChanged = true;
    }
    // Chọn mục
    else if (wParam == VK_RETURN || wParam == VK_SPACE) {
        if (selectedOption == 3) {
            // Chọn "Quay lại Menu"
            currentState = SCREEN_MENU;
            statusMessage = L"";
        }
        else {
            // Tạo tên file tương ứng bằng std::wstring
            std::wstring filename = L"Asset/save_slot_" + std::to_wstring(selectedOption + 1) + L".bin";

            // GỌI HỆ THỐNG LOAD GAME (Cần đảm bảo LoadMatchData nhận const wchar_t*)
            if (LoadMatchData(playState, filename.c_str())) {
                PlaySFX(L"Asset/audio/success.wav");
                currentState = SCREEN_PLAY;
                statusMessage = L"";
            }
            else {
                statusMessage = L"Lỗi: Không tìm thấy hoặc file lưu bị hỏng!";
                PlaySFX(L"Asset/audio/error.wav");
            }

            // Code tạm thời cho mục đích minh họa giao diện:
            statusMessage = L"Lỗi: Module SaveLoad chưa được liên kết!";
        }
        hasChanged = true;
    }
    // Bấm ESC để thoát nhanh về Menu
    else if (wParam == VK_ESCAPE) {
        currentState = SCREEN_MENU;
        statusMessage = L"";
        hasChanged = true;
    }

    return hasChanged;
}

void RenderLoadGameScreen(HDC hdc, int selectedOption, const std::wstring& statusMessage, int screenWidth, int screenHeight) {
    // 1. Phủ màu nền (Clear màn hình cũ)
    RECT rect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::GRAY_LIGHTEST);
    FillRect(hdc, &rect, hBg);
    DeleteObject(hBg);

    // 2. Vẽ tiêu đề màn hình
    int titleY = screenHeight / 4 - 60;
    DrawTextCentered(hdc, L"==================================", titleY, screenWidth, Colour::BLUE_LIGHT);
    DrawTextCentered(hdc, L"TẢI GAME (LOAD GAME)", titleY + 40, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);
    DrawTextCentered(hdc, L"==================================", titleY + 100, screenWidth, Colour::BLUE_LIGHT);

    // 3. Vẽ danh sách các khe lưu game
    int startY = screenHeight / 2 - 20;
    int spacing = 50;

    for (int i = 0; i < TOTAL_LOAD_ITEMS; i++) {
        int currentY = startY + i * spacing;

        if (i == selectedOption) {
            // Mục đang được chọn
            std::wstring highlightedText = L">>  " + std::wstring(loadItems[i]) + L"  <<";
            DrawTextCentered(hdc, highlightedText, currentY, screenWidth, Colour::ORANGE_NORMAL, GlobalFont::Bold);
        }
        else {
            // Mục bình thường
            DrawTextCentered(hdc, loadItems[i], currentY, screenWidth, Colour::GRAY_DARK);
        }
    }

    // 4. Hiển thị thông báo trạng thái (Nếu có lỗi)
    if (!statusMessage.empty()) {
        DrawTextCentered(hdc, statusMessage, startY + TOTAL_LOAD_ITEMS * spacing + 20, screenWidth, Colour::RED_NORMAL, GlobalFont::Bold);
    }

    // 5. Hướng dẫn phím điều khiển
    DrawTextCentered(hdc, L"W/S: Di chuyển | ENTER: Chọn | ESC: Quay lại", screenHeight - 50, screenWidth, Colour::GRAY_NORMAL);
}

void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage, WPARAM wParam) {
    // Bỏ qua nếu không có sự kiện phím (wParam = 0)
    if (wParam == 0) return;

    // Ủy quyền xử lý logic cho ProcessLoadGameInput
    ProcessLoadGameInput(wParam, currentState, playState, selectedOption, statusMessage);
}