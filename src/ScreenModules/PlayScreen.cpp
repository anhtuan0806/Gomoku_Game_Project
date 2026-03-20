#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h" // Sử dụng Engine mới
#include "../GameLogic/BotAI.h"      // Sử dụng AI mới

// Hàm UpdateCountdown cần được định nghĩa trong TimeSystem.h
// Tạm thời mình giả định nó trả về true nếu hết thời gian
extern bool UpdateCountdown(PlayState* state, double dt);

bool UpdatePlayLogic(PlayState* state, double dt) {
    bool needsRedraw = false;

    if (state->status == MATCH_PLAYING) {
        if (UpdateCountdown(state, dt)) {
            switchTurn(state); // GameEngine xử lý
            needsRedraw = true;
        }
    }

    // Lượt của AI (Máy)
    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isP1Turn) {
        int aiRow, aiCol;
        calculateAIMove(state, 1, aiRow, aiCol); // Độ khó 1 (Dễ)

        if (processMove(state, aiRow, aiCol)) { // GameEngine tự đánh, tự check thắng/thua/hòa
            needsRedraw = true;
        }
    }

    return needsRedraw;
}

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState) {
    bool hasChanged = false;

    if (state->status == MATCH_PAUSED) {
        if (wParam == VK_ESCAPE) { state->status = MATCH_PLAYING; hasChanged = true; }
        else if (wParam == 'Q' || wParam == 'q') { currentState = SCREEN_MENU; hasChanged = true; }
        return hasChanged;
    }

    if (state->status == MATCH_FINISHED) {
        if (wParam == 'Y' || wParam == 'y') {
            // Khởi tạo lại ván mới giữ nguyên setting cũ
            initNewMatch(state, state->gameMode, state->matchType, state->boardSize, state->countdownTime);
            hasChanged = true;
        }
        else if (wParam == 'N' || wParam == 'n' || wParam == VK_ESCAPE) {
            currentState = SCREEN_MENU;
            hasChanged = true;
        }
        return hasChanged;
    }

    if (state->status == MATCH_PLAYING) {
        if ((wParam == 'W' || wParam == VK_UP) && state->cursorRow > 0) { state->cursorRow--; hasChanged = true; }
        if ((wParam == 'S' || wParam == VK_DOWN) && state->cursorRow < state->boardSize - 1) { state->cursorRow++; hasChanged = true; }
        if ((wParam == 'A' || wParam == VK_LEFT) && state->cursorCol > 0) { state->cursorCol--; hasChanged = true; }
        if ((wParam == 'D' || wParam == VK_RIGHT) && state->cursorCol < state->boardSize - 1) { state->cursorCol++; hasChanged = true; }

        if (wParam == VK_RETURN || wParam == VK_SPACE) {
            // processMove tự động đổi lượt và cập nhật trạng thái nếu thắng
            if (processMove(state, state->cursorRow, state->cursorCol)) {
                hasChanged = true;
            }
        }

        if (wParam == VK_ESCAPE) {
            state->status = MATCH_PAUSED;
            hasChanged = true;
        }
    }
    return hasChanged;
}

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO) {
    RECT bgRect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::WHITE);
    FillRect(hdc, &bgRect, hBg);
    DeleteObject(hBg);

    int boardPixelSize = state->boardSize * CELL_SIZE;
    int startX = (screenWidth - boardPixelSize) / 2;
    int startY = (screenHeight - boardPixelSize) / 2 + 30;

    std::wstring modeTitle = (state->gameMode == MODE_CARO) ? L"--- CARO ---" : L"--- TIC-TAC-TOE ---";
    DrawTextCentered(hdc, modeTitle, 20, screenWidth, Colour::BLUE_DARK);

    HFONT hOldFont = (HFONT)SelectObject(hdc, GlobalFont::Default);
    SetBkMode(hdc, TRANSPARENT);

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

    // --- SỬ DỤNG HÀM VẼ TỐI ƯU ---
    DrawGameBoard(hdc, state, CELL_SIZE, startX, startY, spriteX, spriteO);

    // Hiệu ứng Bán trong suốt khi PAUSE / KẾT THÚC
    if (state->status == MATCH_PAUSED || state->status == MATCH_FINISHED) {
        Gdiplus::Graphics g(hdc);
        Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(178, 0, 0, 0));
        g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

        int popupW = 500, popupH = 200;
        int popupX = (screenWidth - popupW) / 2;
        int popupY = (screenHeight - popupH) / 2;

        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 255, 255, 255));
        Gdiplus::Pen borderPen(Gdiplus::Color(255, 100, 100, 100), 3);
        g.FillRectangle(&bgBrush, popupX, popupY, popupW, popupH);
        g.DrawRectangle(&borderPen, popupX, popupY, popupW, popupH);
    }

    if (state->status == MATCH_PAUSED) {
        int popupY = (screenHeight - 200) / 2;
        DrawTextCentered(hdc, L"--- GAME TẠM DỪNG ---", popupY + 50, screenWidth, Colour::ORANGE_NORMAL, GlobalFont::Title);
        DrawTextCentered(hdc, L"ESC: Tiếp tục   |   Q: Về Menu", popupY + 130, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
    }
    else if (state->status == MATCH_FINISHED) {
        int popupY = (screenHeight - 200) / 2;
        std::wstring winMsg;
        COLORREF winColor;

        if (state->winner == CELL_PLAYER1) { winMsg = p1NameW + L" CHIẾN THẮNG!"; winColor = Colour::RED_NORMAL; }
        else if (state->winner == CELL_PLAYER2) { winMsg = p2NameW + L" CHIẾN THẮNG!"; winColor = Colour::BLUE_NORMAL; }
        else { winMsg = L"HAI BÊN HÒA NHAU!"; winColor = Colour::ORANGE_NORMAL; }

        DrawTextCentered(hdc, winMsg, popupY + 50, screenWidth, winColor, GlobalFont::Title);
        DrawTextCentered(hdc, L"Nhấn 'Y' để Chơi lại hoặc 'N' để Về Menu", popupY + 130, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
    }
}

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam) {
    if (wParam == 0) return;
    ProcessPlayInput(wParam, state, currentState);
}