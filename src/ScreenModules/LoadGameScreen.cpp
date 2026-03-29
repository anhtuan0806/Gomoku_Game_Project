#include "LoadGameScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/SaveLoadSystem.h" 
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/Localization.h"
#include "../ApplicationTypes/GameConstants.h"

const int BACK_OPTION = 5;
const int TOTAL_LOAD_ITEMS = 6;

bool ProcessLoadGameInput(WPARAM wParam, ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage) {
    bool hasChanged = false;

    if (wParam == 'W' || wParam == VK_UP) {
        selectedOption = (selectedOption - 1 + TOTAL_LOAD_ITEMS) % TOTAL_LOAD_ITEMS;
        statusMessage = L"";
        hasChanged = true;
    }
    else if (wParam == 'S' || wParam == VK_DOWN) {
        selectedOption = (selectedOption + 1) % TOTAL_LOAD_ITEMS;
        statusMessage = L"";
        hasChanged = true;
    }
    else if (wParam == VK_RETURN || wParam == VK_SPACE) {
        if (selectedOption == BACK_OPTION) {
            currentState = SCREEN_MENU;
            statusMessage = L"";
        }
        else {
            // 1. Lấy đúng đường dẫn từ SaveLoadSystem
            std::wstring filename = GetSavePath(selectedOption + 1);

            // 2. Kiểm tra file tồn tại trước
            if (!CheckSaveExists(selectedOption + 1)) {
                statusMessage = L"Lỗi: Slot này chưa có dữ liệu!";
                PlaySFX(L"Asset/audio/error.wav");
            }
            else {
                // 3. Thực hiện Load
                if (LoadMatchData(playState, filename.c_str())) {
                    PlaySFX(L"Asset/audio/success.wav");

                    // CẬP NHẬT TRẠNG THÁI TRẬN ĐẤU (Rất quan trọng)
                    playState->status = MATCH_PLAYING;

                    currentState = SCREEN_PLAY;
                    statusMessage = L"";
                }
                else {
                    statusMessage = L"Lỗi: Không thể đọc file lưu!";
                    PlaySFX(L"Asset/audio/error.wav");
                }
            }
        }
        hasChanged = true;
    }
    else if (wParam == VK_ESCAPE) {
        currentState = SCREEN_MENU;
        hasChanged = true;
    }

    return hasChanged;
}

void RenderLoadGameScreen(HDC hdc, int selectedOption, const std::wstring& statusMessage, int screenWidth, int screenHeight) {
    // Vẽ nền
    RECT rect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::GRAY_LIGHTEST);
    FillRect(hdc, &rect, hBg);
    DeleteObject(hBg);

    // Vẽ tiêu đề
    int titleY = 80;
    DrawTextCentered(hdc, L"--- DANH SÁCH BẢN LƯU ---", titleY, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

    // Vẽ các Slot
    int startY = 180;
    int spacing = 50;

    for (int i = 0; i < TOTAL_LOAD_ITEMS; i++) {
        std::wstring itemText;
        COLORREF color = Colour::GRAY_DARK;
        HFONT font = GlobalFont::Default;

        if (i == BACK_OPTION) {
            itemText = L"Quay lại Menu";
        }
        else {
            bool exists = CheckSaveExists(i + 1);
            itemText = L"Slot " + std::to_wstring(i + 1) + (exists ? L" [Dữ liệu sẵn sàng]" : L" [Trống]");
        }

        if (i == selectedOption) {
            itemText = L"> " + itemText + L" <";
            color = Colour::ORANGE_NORMAL;
            font = GlobalFont::Bold;
        }

        DrawTextCentered(hdc, itemText, startY + i * spacing, screenWidth, color, font);
    }

    // Thông báo lỗi
    if (!statusMessage.empty()) {
        DrawTextCentered(hdc, statusMessage, startY + TOTAL_LOAD_ITEMS * spacing + 30, screenWidth, Colour::RED_NORMAL, GlobalFont::Bold);
    }

    DrawTextCentered(hdc, L"W/S: Di chuyển | ENTER: Tải game | ESC: Thoát", screenHeight - 60, screenWidth, Colour::GRAY_NORMAL);
}

void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage, WPARAM wParam) {
    // Bỏ qua nếu không có sự kiện phím (wParam = 0)
    if (wParam == 0) return;

    // Ủy quyền xử lý logic cho ProcessLoadGameInput
    ProcessLoadGameInput(wParam, currentState, playState, selectedOption, statusMessage);
}