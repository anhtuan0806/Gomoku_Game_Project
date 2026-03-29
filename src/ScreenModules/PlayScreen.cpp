#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"
#include "../GameLogic/BotAI.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/SaveLoadSystem.h"
#include "../SystemModules/ConfigLoader.h"
#include "../SystemModules/Localization.h"
#include "../ApplicationTypes/GameConstants.h"

extern bool UpdateCountdown(PlayState* state, double dt);

bool UpdatePlayLogic(PlayState* state, double dt) {
    bool needsRedraw = false;

    if (state->status == MATCH_PLAYING) {
        // Nếu một giây trôi qua, UpdateCountdown trả về true
        if (UpdateCountdown(state, dt)) {
            needsRedraw = true;

            // Nếu sau khi trừ giây mà thời gian bằng 0 -> Hết lượt, đổi người chơi
            if (state->timeRemaining <= 0) {
                switchTurn(state);
            }
        }
    }

    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isP1Turn) {
        int aiRow, aiCol;
        // Truyền độ khó đã lưu trong state vào AI
        calculateAIMove(state, state->difficulty, aiRow, aiCol);
        if (processMove(state, aiRow, aiCol)) needsRedraw = true;
    }

    return needsRedraw;
}

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState, GameConfig* config) {
    bool hasChanged = false;

    if (state->status == MATCH_PAUSED) {
        // TRƯỜNG HỢP 1: ĐANG NHẬP TÊN 
        if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            if (wParam == VK_ESCAPE) {
                g_CurrentSubMenu = SUB_SAVE_SELECT;
                return true;
            }
            if (wParam == VK_RETURN) {
                if (g_SaveNameInput.empty()) {
                    g_SaveNameInput = L"NEW_SAVE";
                }
                else if (SaveMatchData(state, GetSavePath(g_SaveSlotSelected + 1).c_str())) {
                    PlaySFX(L"Asset/audio/success.wav");
                    g_CurrentSubMenu = SUB_MAIN; 
                    g_SaveNameInput = L"";
                }
                return true;
            }
            if (wParam == VK_BACK) {
                if (!g_SaveNameInput.empty()) { 
                    g_SaveNameInput.pop_back(); hasChanged = true; 
                }
            }
            else if (g_SaveNameInput.length() < MAX_SAVE_NAME_LEN) {
                if ((wParam >= '0' && wParam <= '9') || (wParam >= 'A' && wParam <= 'Z') || wParam == VK_SPACE) {
                    g_SaveNameInput += (wchar_t)wParam;
                    hasChanged = true;
                }
            }
            return hasChanged;
        }

        // TRƯỜNG HỢP 2: CHỌN SLOT LƯU
        if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            if (wParam == VK_ESCAPE) { g_CurrentSubMenu = SUB_MAIN; return true; }
            if (wParam == 'W' || wParam == VK_UP) {
                g_SaveSlotSelected = (g_SaveSlotSelected - 1 < 0) ? MAX_SAVE_SLOTS - 1 : g_SaveSlotSelected - 1;
                return true;
            }
            if (wParam == 'S' || wParam == VK_DOWN) {
                g_SaveSlotSelected = (g_SaveSlotSelected + 1 >= MAX_SAVE_SLOTS) ? 0 : g_SaveSlotSelected + 1;
                return true;
            }
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                g_CurrentSubMenu = SUB_SAVE_NAME_ENTRY;
                g_SaveNameInput = L"";
                return true;
            }
            return false;
        }

        // TRƯỜNG HỢP 3: MENU PAUSE CHÍNH
        if (wParam == VK_ESCAPE) { 
            state->status = MATCH_PLAYING;     
            return true; 
        }

        if (wParam == 'W' || wParam == VK_UP) {
            g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            hasChanged = true;
        }
        else if (wParam == 'S' || wParam == VK_DOWN) {
            g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            hasChanged = true;
        }
        else if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_RIGHT || wParam == VK_LEFT) {
            int dir = (wParam == VK_LEFT) ? -1 : 1;
            switch (g_PauseSelected) {
            case 0: 
                state->status = MATCH_PLAYING; 
                break; 
            case 1: 
                config->isBgmEnabled = !config->isBgmEnabled; 
                if (!config->isBgmEnabled) { 
                    StopBGM(); 
                }
                break; 
            case 2:
                config->sfxVolume += dir * 10;
                if (config->sfxVolume > 100) {
                    config->sfxVolume = 100;
                }
                if (config->sfxVolume < 0) {
                    config->sfxVolume = 0;
                }
                break;
            case 3:
                config->currentLang = (config->currentLang == APP_LANG_VI) ? APP_LANG_EN : APP_LANG_VI;
                LoadLanguageFile(config->currentLang);
                break;
            case 4: 
                g_CurrentSubMenu = SUB_SAVE_SELECT; 
                break;
            case 5: 
                SaveConfig(config, "Asset/config.ini");
                currentState = SCREEN_MENU;
                ResetPlayScreenStatics(); 
                break;
            }
            hasChanged = true;
        }
        return hasChanged;
    }

    if (state->status == MATCH_PLAYING) {
        if (wParam == VK_ESCAPE) {
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }
        if ((wParam == 'W' || wParam == VK_UP) && state->cursorRow > 0) { 
            state->cursorRow--; 
            hasChanged = true; 
        }
        if ((wParam == 'S' || wParam == VK_DOWN) && state->cursorRow < state->boardSize - 1) { 
            state->cursorRow++; 
            hasChanged = true; 
        }
        if ((wParam == 'A' || wParam == VK_LEFT) && state->cursorCol > 0) { 
            state->cursorCol--; 
            hasChanged = true; 
        }
        if ((wParam == 'D' || wParam == VK_RIGHT) && state->cursorCol < state->boardSize - 1) { 
            state->cursorCol++; 
            hasChanged = true; 
        }
        if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (processMove(state, state->cursorRow, state->cursorCol)) { 
                hasChanged = true; 
            }
        }
    }

    if (state->status == MATCH_FINISHED) {
        if (wParam == 'Y' || wParam == 'y') {
			// Reset về cấu hình trận đấu ban đầu để bắt đầu ván mới
			startNextRound(state);
            hasChanged = true;
        }
        else if (wParam == 'N' || wParam == 'n' || wParam == VK_ESCAPE) {
            currentState = SCREEN_MENU;
            hasChanged = true;
        }
    }

    return hasChanged;
}

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO, const GameConfig* config) {
    // 1. Vẽ bàn cờ và thông tin trận đấu 
    RECT bgRect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::WHITE);
    FillRect(hdc, &bgRect, hBg);
    DeleteObject(hBg);

    int boardPixelSize = state->boardSize * CELL_SIZE;
    int startX = (screenWidth - boardPixelSize) / 2;
    int startY = (screenHeight - boardPixelSize) / 2 + 30;
    DrawGameBoard(hdc, state, CELL_SIZE, startX, startY, spriteX, spriteO);

	HFONT hOldFont = (HFONT)SelectObject(hdc, GlobalFont::Default);
    // Điểm P1
    std::string n1 = state->p1.name;
    std::wstring p1Text = std::wstring(n1.begin(), n1.end()) + L" (X): " + std::to_wstring(state->p1.score);
    SetTextColor(hdc, Colour::RED_NORMAL);
    RECT rLeft = { 50, 60, screenWidth / 2, 100 };
    DrawTextW(hdc, p1Text.c_str(), -1, &rLeft, DT_LEFT | DT_SINGLELINE);

    // Điểm P2
    std::string n2 = state->p2.name;
    std::wstring p2Text = std::wstring(n2.begin(), n2.end()) + L" (O): " + std::to_wstring(state->p2.score);
    SetTextColor(hdc, Colour::BLUE_NORMAL);
    RECT rRight = { screenWidth / 2, 60, screenWidth - 50, 100 };
    DrawTextW(hdc, p2Text.c_str(), -1, &rRight, DT_RIGHT | DT_SINGLELINE);

    std::wstring p1NameW(state->p1.name, state->p1.name + strlen(state->p1.name));
    std::wstring p2NameW(state->p2.name, state->p2.name + strlen(state->p2.name));
    std::wstring turnText = L"Lượt của: " + (state->isP1Turn ? p1NameW : p2NameW);
    COLORREF turnColor = state->isP1Turn ? Colour::RED_NORMAL : Colour::BLUE_NORMAL;
    DrawTextCentered(hdc, turnText, 60, screenWidth, turnColor);

    std::wstring timeText = L"Thời gian: " + std::to_wstring(state->timeRemaining) + L"s";
    DrawTextCentered(hdc, timeText, 90, screenWidth, Colour::GRAY_DARK);
    SelectObject(hdc, hOldFont);

    // 2. Lớp phủ Pause Menu
    if (state->status == MATCH_PAUSED) {
        Gdiplus::Graphics g(hdc);
        Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(200, 0, 0, 0));
        g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

        int menuW = 450, menuH = 450;
        int menuX = (screenWidth - menuW) / 2;
        int menuY = (screenHeight - menuH) / 2;
        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 255, 255, 255));
        g.FillRectangle(&bgBrush, menuX, menuY, menuW, menuH);

        if (g_CurrentSubMenu == SUB_MAIN) {
            // VẼ MENU CHÍNH
            DrawTextCentered(hdc, L"--- TẠM DỪNG ---", menuY + 30, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);
            const wchar_t* labels[] = { L"Tiếp tục", L"Nhạc nền: ", L"Âm lượng: ", L"Ngôn ngữ: ", L"Lưu ván đấu", L"Về Menu chính" };
            for (int i = 0; i < TOTAL_PAUSE_ITEMS; i++) {
                std::wstring itemText = labels[i];
                // Thêm giá trị động cho các setting
                if (i == 1) itemText += (config->isBgmEnabled ? L"BẬT" : L"TẮT");
                if (i == 2) itemText += std::to_wstring(config->sfxVolume) + L"%";
                if (i == 3) itemText += (config->currentLang == APP_LANG_VI ? L"Tiếng Việt" : L"English");

                COLORREF color = (i == g_PauseSelected) ? Colour::ORANGE_NORMAL : Colour::GRAY_DARK;
                HFONT font = (i == g_PauseSelected) ? GlobalFont::Bold : GlobalFont::Default;

                DrawTextCentered(hdc, itemText, menuY + 110 + i * 50, screenWidth, color, font);
            }
        }
        else if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            // VẼ MENU CHỌN SLOT LƯU
            DrawTextCentered(hdc, L"--- CHỌN VỊ TRÍ LƯU ---", menuY + 30, screenWidth, Colour::ORANGE_DARK, GlobalFont::Title);

            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                bool exists = CheckSaveExists(i + 1);
                std::wstring slotLabel = L"Slot " + std::to_wstring(i + 1) + (exists ? L" [Đã lưu]" : L" [Trống]");

                if (slotLabel.length() > MAX_SAVE_NAME_LEN) slotLabel = slotLabel.substr(0, MAX_SAVE_NAME_LEN - 3) + L"...";

                COLORREF color = (i == g_SaveSlotSelected) ? Colour::RED_NORMAL : Colour::GRAY_DARK;
                HFONT font = (i == g_SaveSlotSelected) ? GlobalFont::Bold : GlobalFont::Default;
                DrawTextCentered(hdc, slotLabel, menuY + 110 + i * 50, screenWidth, color, font);
            }
            DrawTextCentered(hdc, L"Nhấn ENTER để đặt tên", menuY + 380, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
            DrawTextCentered(hdc, L"ESC: Quay lại", menuY + 380, screenWidth, Colour::GRAY_NORMAL, GlobalFont::Default);
        }
        else if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            // GIAO DIỆN NHẬP TÊN
            DrawTextCentered(hdc, L"NHẬP TÊN FILE LƯU", menuY + 50, screenWidth, Colour::BLUE_DARK, GlobalFont::Title);

            // Vẽ khung nhập liệu (Text Box giả)
            int boxW = 300, boxH = 40;
            int boxX = (screenWidth - boxW) / 2;
            int boxY = menuY + 150;

            // Vẽ hình chữ nhật làm background cho text
            HBRUSH hBoxBrush = CreateSolidBrush(RGB(240, 240, 240));
            RECT rectBox = { boxX, boxY, boxX + boxW, boxY + boxH };
            FillRect(hdc, &rectBox, hBoxBrush);
            DeleteObject(hBoxBrush);

            // Hiển thị nội dung người dùng đang gõ
            std::wstring displayText = g_SaveNameInput + L"_"; // Thêm dấu gạch dưới làm con trỏ nhấp nháy giả
            DrawTextCentered(hdc, displayText.c_str(), boxY + 8, screenWidth, Colour::BLACK, GlobalFont::Bold);

            DrawTextCentered(hdc, L"Nhấn ENTER để xác nhận", menuY + 250, screenWidth, Colour::GREEN_DARK, GlobalFont::Default);
            DrawTextCentered(hdc, L"ESC để quay lại", menuY + 300, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
        }
    }
    else if (state->status == MATCH_FINISHED) {
        int popupY = (screenHeight - 200) / 2;
        std::wstring winMsg;
        COLORREF winColor;

        if (state->winner == CELL_PLAYER1) { 
            winMsg = p1NameW + L" CHIẾN THẮNG!"; 
            winColor = Colour::RED_NORMAL; 
        }
        else if (state->winner == CELL_PLAYER2) { 
            winMsg = p2NameW + L" CHIẾN THẮNG!"; 
            winColor = Colour::BLUE_NORMAL; 
        }
        else { 
            winMsg = L"HAI BÊN HÒA NHAU!"; 
            winColor = Colour::ORANGE_NORMAL; 
        }

        DrawTextCentered(hdc, winMsg, popupY + 50, screenWidth, winColor, GlobalFont::Title);
        DrawTextCentered(hdc, L"Nhấn 'Y' để Chơi lại hoặc 'N' để Về Menu", popupY + 130, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
    }
}

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam, GameConfig* config) {
    if (wParam == 0) { 
        return; 
    }
    ProcessPlayInput(wParam, state, currentState, config);
}

void ResetPlayScreenStatics() {
    g_CurrentSubMenu = SUB_MAIN;
    g_PauseSelected = 0;
    g_SaveSlotSelected = 0;
    g_SaveNameInput = L"";
}