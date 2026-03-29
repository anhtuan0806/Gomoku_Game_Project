#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"
#include "../GameLogic/BotAI.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/SaveLoadSystem.h"
#include "../SystemModules/ConfigLoader.h"
#include "../SystemModules/Localization.h"

extern bool UpdateCountdown(PlayState* state, double dt);

// Biến cục bộ quản lý lựa chọn trong Menu Pause
static int g_PauseSelected = 0;
const int TOTAL_PAUSE_ITEMS = 6;

bool UpdatePlayLogic(PlayState* state, double dt) {
    bool needsRedraw = false;
    if (state->status == MATCH_PLAYING) {
        if (UpdateCountdown(state, dt)) {
            switchTurn(state);
            needsRedraw = true;
        }
    }
    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isP1Turn) {
        int aiRow, aiCol;
        calculateAIMove(state, 1, aiRow, aiCol);
        if (processMove(state, aiRow, aiCol)) needsRedraw = true;
    }
    return needsRedraw;
}

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState, GameConfig* config) {
    bool hasChanged = false;

    // --- LOGIC KHI ĐANG PAUSE ---
    if (state->status == MATCH_PAUSED) {
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
            hasChanged = true;
            int dir = (wParam == VK_LEFT) ? -1 : 1;

            switch (g_PauseSelected) {
            case 0: // Tiếp tục
                state->status = MATCH_PLAYING;
                break;
            case 1: // Nhạc nền (BGM)
                config->isBgmEnabled = !config->isBgmEnabled;
                if (!config->isBgmEnabled) StopBGM();
                // else PlayBGM("Asset/audio/bgm_menu.wav");
                break;
            case 2: // Âm lượng (Adjust Volume - Ví dụ chỉnh SFX)
                config->sfxVolume += dir * 10;
                if (config->sfxVolume > 100) config->sfxVolume = 100;
                if (config->sfxVolume < 0) config->sfxVolume = 0;
                break;
            case 3: // Ngôn ngữ
                config->currentLang = (config->currentLang == APP_LANG_VI) ? APP_LANG_EN : APP_LANG_VI;
                LoadLanguageFile(config->currentLang);
                break;
            case 4: // Lưu Game
                if (SaveMatchData(state, L"Asset/save_auto.bin")) {
                    PlaySFX(L"Asset/audio/success.wav");
                }
                break;
            case 5: // Thoát
                SaveConfig(config, "Asset/config.ini");
                currentState = SCREEN_MENU;
                break;
            }
        }
        return hasChanged;
    }

    // --- LOGIC KHI ĐANG CHƠI ---
    if (state->status == MATCH_PLAYING) {
        if (wParam == VK_ESCAPE) {
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }
        // ... (Giữ nguyên logic di chuyển cursor Row/Col và processMove)
        if ((wParam == 'W' || wParam == VK_UP) && state->cursorRow > 0) { state->cursorRow--; hasChanged = true; }
        if ((wParam == 'S' || wParam == VK_DOWN) && state->cursorRow < state->boardSize - 1) { state->cursorRow++; hasChanged = true; }
        if ((wParam == 'A' || wParam == VK_LEFT) && state->cursorCol > 0) { state->cursorCol--; hasChanged = true; }
        if ((wParam == 'D' || wParam == VK_RIGHT) && state->cursorCol < state->boardSize - 1) { state->cursorCol++; hasChanged = true; }
        if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (processMove(state, state->cursorRow, state->cursorCol)) hasChanged = true;
        }
    }

    // Logic khi kết thúc (Win/Loss)
    if (state->status == MATCH_FINISHED) {
        if (wParam == 'Y' || wParam == 'y') {
            initNewMatch(state, state->gameMode, state->matchType, state->boardSize, state->countdownTime);
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
    // 1. Vẽ bàn cờ và thông tin trận đấu (Giữ nguyên phần vẽ cũ)
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
        Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(200, 0, 0, 0)); // Phủ đen mờ toàn màn hình
        g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

        int menuW = 400, menuH = 450;
        int menuX = (screenWidth - menuW) / 2;
        int menuY = (screenHeight - menuH) / 2;

        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 255, 255, 255));
        g.FillRectangle(&bgBrush, menuX, menuY, menuW, menuH);

        DrawTextCentered(hdc, L"--- TẠM DỪNG ---", menuY + 30, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

        const wchar_t* labels[] = {
            L"Tiếp tục (Resume)",
            L"Nhạc nền (BGM): ",
            L"Âm lượng SFX: ",
            L"Ngôn ngữ: ",
            L"Lưu ván đấu (Save)",
            L"Về Menu chính (Exit)"
        };

        for (int i = 0; i < TOTAL_PAUSE_ITEMS; i++) {
            std::wstring itemText = labels[i];
            // Thêm giá trị động cho các setting
            if (i == 1) { 
                itemText += (config->isBgmEnabled ? L"BẬT" : L"TẮT"); 
            }
            if (i == 2) { 
                itemText += std::to_wstring(config->sfxVolume) + L"%"; 
            }
            if (i == 3) { 
                itemText += (config->currentLang == APP_LANG_VI ? L"Tiếng Việt" : L"English"); 
            }

            COLORREF color = (i == g_PauseSelected) ? Colour::ORANGE_NORMAL : Colour::GRAY_DARK;
            HFONT font = (i == g_PauseSelected) ? GlobalFont::Bold : GlobalFont::Default;

            DrawTextCentered(hdc, itemText, menuY + 110 + i * 50, screenWidth, color, font);
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