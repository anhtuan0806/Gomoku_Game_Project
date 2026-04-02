#include "MatchConfigScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"

const int TOTAL_CONFIG_ITEMS = 8;
static std::wstring editName1 = L"Player 1";
static std::wstring editName2 = L"Player 2";
static bool isEditingP1 = false;
static bool isEditingP2 = false;

void UpdateMatchConfigScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, WPARAM wParam) {
    if (wParam == 0) return;

    // --- TRƯỜNG HỢP 1: ĐANG NHẬP TÊN ---
    if (isEditingP1 || isEditingP2) {
        std::wstring& targetName = isEditingP1 ? editName1 : editName2;

        if (wParam == VK_RETURN) { // Nhấn Enter để hoàn tất nhập
            isEditingP1 = isEditingP2 = false;
            return;
        }
        if (wParam == VK_BACK) { // Xóa ký tự
            if (!targetName.empty()) targetName.pop_back();
            return;
        }
        // Nhận các ký tự chữ và số (cơ bản)
        if (targetName.length() < 12 && ((wParam >= 'A' && wParam <= 'Z') || (wParam >= '0' && wParam <= '9') || wParam == VK_SPACE)) {
            targetName += (wchar_t)wParam;
        }
        return;
    }

    // --- TRƯỜNG HỢP 2: DI CHUYỂN MENU ---
    if (wParam == 'W' || wParam == VK_UP) {
        selectedOption = (selectedOption - 1 + TOTAL_CONFIG_ITEMS) % TOTAL_CONFIG_ITEMS;
    }
    else if (wParam == 'S' || wParam == VK_DOWN) {
        selectedOption = (selectedOption + 1) % TOTAL_CONFIG_ITEMS;
    }

    // Xác định hướng thay đổi thông số (Trái/Phải hoặc A/D)
    int dir = (wParam == 'D' || wParam == VK_RIGHT) ? 1 : ((wParam == 'A' || wParam == VK_LEFT) ? -1 : 0);

    switch (selectedOption) {
    case 0: // Loại cờ
        if (dir != 0) playState->gameMode = (playState->gameMode == MODE_CARO) ? MODE_TIC_TAC_TOE : MODE_CARO;
        break;
    case 1: // Chế độ chơi
        if (dir != 0) playState->matchType = (playState->matchType == MATCH_PVP) ? MATCH_PVE : MATCH_PVP;
        break;
    case 2: // Độ khó (Chỉ khi chơi với Máy)
        if (playState->matchType == MATCH_PVE && dir != 0) {
            playState->difficulty += dir;
            if (playState->difficulty < 1) playState->difficulty = 3;
            if (playState->difficulty > 3) playState->difficulty = 1;
        }
        break;
    case 3: // Thời gian lượt (10s - 60s)
        if (dir != 0) {
            playState->countdownTime += dir * 5;
            if (playState->countdownTime < 10) playState->countdownTime = 10;
            if (playState->countdownTime > 60) playState->countdownTime = 60;
        }
        break;
    case 4: // Thể thức Bo (1, 3, 5)
        if (dir != 0) {
            playState->targetScore += dir * 2;
            if (playState->targetScore < 1) playState->targetScore = 5;
            if (playState->targetScore > 5) playState->targetScore = 1;
        }
        break;
    case 5: // Sửa tên P1
        if (wParam == VK_RETURN) isEditingP1 = true;
        break;
    case 6: // Sửa tên P2
        if (playState->matchType == MATCH_PVP && wParam == VK_RETURN) isEditingP2 = true;
        break;
    case 7: // BẮT ĐẦU
        if (wParam == VK_RETURN) {
            playState->p1.name = editName1;

            if (playState->matchType == MATCH_PVE) {
                std::string botName = (playState->difficulty == 1) ? "Bot De" : (playState->difficulty == 2 ? "Bot Vua" : "Bot Kho");
                playState->p2.name = std::wstring(botName.begin(), botName.end());
            }
            else {
                playState->p2.name = editName2;
            }

            int bSize = (playState->gameMode == MODE_CARO) ? 15 : 3;
            initNewMatch(playState,
                playState->gameMode,
                playState->matchType,
                bSize,
                playState->countdownTime,
                playState->difficulty,
                playState->targetScore,
                15); // Ví dụ mặc định 15 phút cả trận  
            currentState = SCREEN_PLAY;
        }
        break;
    }
}

void RenderMatchConfigScreen(HDC hdc, int selectedOption, const PlayState* config, int screenWidth, int screenHeight) {
    RECT rect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::GRAY_LIGHTEST);
    FillRect(hdc, &rect, hBg); DeleteObject(hBg);

    DrawTextCentered(hdc, L"--- THIẾT LẬP TRẬN ĐẤU ---", 50, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

    std::wstring labels[] = {
        L"Chế độ: " + std::wstring(config->gameMode == MODE_CARO ? L"Caro 15x15" : L"Tic-Tac-Toe 3x3"),
        L"Đối thủ: " + std::wstring(config->matchType == MATCH_PVP ? L"Người vs Người" : L"Người vs Máy"),
        L"Độ khó AI: " + std::wstring(config->difficulty == 1 ? L"Dễ" : (config->difficulty == 2 ? L"Trung bình" : L"Khó")),
        L"TG mỗi lượt: " + std::to_wstring(config->countdownTime) + L"s",
        L"Thể thức: Bo" + std::to_wstring(config->targetScore),
        L"Tên P1: " + editName1 + (isEditingP1 ? L"_" : L""),
        L"Tên P2: " + (config->matchType == MATCH_PVE ? L"[BOT]" : editName2 + (isEditingP2 ? L"_" : L"")),
        L"== BẮT ĐẦU CHIẾN ĐẤU =="
    };

    for (int i = 0; i < TOTAL_CONFIG_ITEMS; i++) {
        COLORREF color = (i == selectedOption) ? Colour::ORANGE_NORMAL : Colour::GRAY_DARK;
        if (i == 2 && config->matchType == MATCH_PVP) color = Colour::GRAY_LIGHT; // Vô hiệu hóa dòng độ khó nếu chơi PvP
        DrawTextCentered(hdc, labels[i], 150 + i * 50, screenWidth, color, (i == selectedOption ? GlobalFont::Bold : GlobalFont::Default));
    }
    DrawTextCentered(hdc, L"A/D: Thay đổi | Enter: Sửa tên/Bắt đầu | ESC: Quay lại", screenHeight - 60, screenWidth, Colour::GRAY_NORMAL);
}