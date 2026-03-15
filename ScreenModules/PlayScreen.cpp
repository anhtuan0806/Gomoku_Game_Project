#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/MatchRules.h"
#include "../GameLogic/StateUpdater.h"
#include "../GameLogic/AI_PvE.h"
#include "../SystemModules/TimeSystem.h"
#include "../SystemModules/AudioSystem.h"

static void HandleMoveResult(PlayState* state) {
    int winStatus = CheckWinCondition(state);
    if (winStatus != 2) {
        state->status = MATCH_FINISHED;
        state->winner = winStatus;
        if (winStatus == -1) state->player1.score++;
        else if (winStatus == 1) state->player2.score++;
    }
    else {
        SwitchTurn(state);
    }
}

bool UpdatePlayLogic(PlayState* state, double dt) {
    bool needsRedraw = false;

    // 1. Cập nhật đếm ngược nếu đang chơi
    if (state->status == MATCH_PLAYING) {
        if (UpdateCountdown(state, dt)) {
            SwitchTurn(state); // Hết giờ thì mất lượt
            needsRedraw = true;
        }
    }

    // 2. Lượt của AI (Máy)
    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isPlayer1Turn) {
        uint8_t aiX, aiY;
        CalculateAIMove(state, aiX, aiY);

        if (ProcessMove(state, aiX, aiY)) {
            // PlaySFX(L"Asset/audio/place_piece.wav"); // Đảm bảo thư viện âm thanh dùng wchar_t
            HandleMoveResult(state);
            needsRedraw = true;
        }
    }

    return needsRedraw;
}

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState) {
    bool hasChanged = false;

    // --- NẾU ĐANG TẠM DỪNG (PAUSE) ---
    if (state->status == MATCH_PAUSED) {
        if (wParam == VK_ESCAPE) {
            state->status = MATCH_PLAYING;
            hasChanged = true;
        }
        else if (wParam == 'Q' || wParam == 'q') {
            currentState = SCREEN_MENU;
            hasChanged = true;
        }
        return hasChanged;
    }

    // --- NẾU ĐÃ KẾT THÚC (GAME OVER) ---
    if (state->status == MATCH_FINISHED) {
        if (wParam == 'Y' || wParam == 'y') {
            // InitNewMatch(state, state->gameMode, state->matchType, state->boardSize, state->countdownTime);
            hasChanged = true;
        }
        else if (wParam == 'N' || wParam == 'n' || wParam == VK_ESCAPE) {
            currentState = SCREEN_MENU;
            hasChanged = true;
        }
        return hasChanged;
    }

    // --- NẾU ĐANG CHƠI THÔNG THƯỜNG ---
    if (state->status == MATCH_PLAYING) {
        // Di chuyển con trỏ
        if ((wParam == 'W' || wParam == 'w' || wParam == VK_UP) && state->cursor.y > 0) { state->cursor.y--; hasChanged = true; }
        if ((wParam == 'S' || wParam == 's' || wParam == VK_DOWN) && state->cursor.y < state->boardSize - 1) { state->cursor.y++; hasChanged = true; }
        if ((wParam == 'A' || wParam == 'a' || wParam == VK_LEFT) && state->cursor.x > 0) { state->cursor.x--; hasChanged = true; }
        if ((wParam == 'D' || wParam == 'd' || wParam == VK_RIGHT) && state->cursor.x < state->boardSize - 1) { state->cursor.x++; hasChanged = true; }

        // Nhấn Enter hoặc Space để đánh cờ
        if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (ProcessMove(state, state->cursor.x, state->cursor.y)) {
                // PlaySFX(L"Asset/audio/place_piece.wav");
                HandleMoveResult(state);
                hasChanged = true;
            }
        }

        // Tạm dừng
        if (wParam == VK_ESCAPE) {
            state->status = MATCH_PAUSED;
            hasChanged = true;
        }
    }

    return hasChanged;
}

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO) {
    // 0. Phủ nền bàn cờ
    RECT bgRect = { 0, 0, screenWidth, screenHeight };
    HBRUSH hBg = CreateSolidBrush(Colour::WHITE);
    FillRect(hdc, &bgRect, hBg);
    DeleteObject(hBg);

    // 1. Tính toán tọa độ lưới để căn giữa
    int boardPixelSize = state->boardSize * CELL_SIZE;
    int startX = (screenWidth - boardPixelSize) / 2;
    int startY = (screenHeight - boardPixelSize) / 2 + 30;

    // 2. Vẽ UI bên trên (Chuẩn Unicode)
    std::wstring modeTitle = (state->gameMode == MODE_CARO) ? L"--- CARO ---" : L"--- TIC-TAC-TOE ---";
    DrawTextCentered(hdc, modeTitle, 20, screenWidth, Colour::BLUE_DARK);

    // Dùng DrawTextW chuẩn để căn lề Trái/Phải cho điểm số
    HFONT hOldFont = (HFONT)SelectObject(hdc, GlobalFont::Default);
    SetBkMode(hdc, TRANSPARENT);

    // P1 (Trái)
    std::string n1 = state->player1.name;
    std::wstring p1Text = std::wstring(n1.begin(), n1.end()) + L" (X): " + std::to_wstring(state->player1.score);
    SetTextColor(hdc, Colour::RED_NORMAL);
    RECT rLeft = { 50, 60, screenWidth / 2, 100 };
    DrawTextW(hdc, p1Text.c_str(), -1, &rLeft, DT_LEFT | DT_SINGLELINE);

    // P2 (Phải)
    std::string n2 = state->player2.name;
    std::wstring p2Text = std::wstring(n2.begin(), n2.end()) + L" (O): " + std::to_wstring(state->player2.score);
    SetTextColor(hdc, Colour::BLUE_NORMAL);
    RECT rRight = { screenWidth / 2, 60, screenWidth - 50, 100 };
    DrawTextW(hdc, p2Text.c_str(), -1, &rRight, DT_RIGHT | DT_SINGLELINE);

    // Lượt đi & Thời gian
    std::wstring p1NameW(state->player1.name, state->player1.name + strlen(state->player1.name));
    std::wstring p2NameW(state->player2.name, state->player2.name + strlen(state->player2.name));
    std::wstring turnText = L"Lượt của: " + (state->isPlayer1Turn ? p1NameW : p2NameW);
    COLORREF turnColor = state->isPlayer1Turn ? Colour::RED_NORMAL : Colour::BLUE_NORMAL;
    DrawTextCentered(hdc, turnText, 60, screenWidth, turnColor);

    std::wstring timeText = L"Thời gian: " + std::to_wstring(state->timeRemaining) + L"s";
    DrawTextCentered(hdc, timeText, 90, screenWidth, Colour::GRAY_DARK);

    SelectObject(hdc, hOldFont); // Phục hồi Font

    // 3. Vẽ Bàn Cờ
    DrawGrid(hdc, state->boardSize, CELL_SIZE, startX, startY);

    // 4. Batch Rendering Quân Cờ & Highlight bằng GDI+
    {
        Gdiplus::Graphics g(hdc);
        // Không gọi SetInterpolationMode vì ảnh đã được Pre-scale từ đầu

        for (int y = 0; y < state->boardSize; y++) {
            for (int x = 0; x < state->boardSize; x++) {
                if (state->board[y][x].c == 0) continue;

                int cellPx = startX + x * CELL_SIZE;
                int cellPy = startY + y * CELL_SIZE;

                if (state->board[y][x].c == 1) DrawSprite(g, spriteX, cellPx, cellPy, CELL_SIZE);
                else if (state->board[y][x].c == 2) DrawSprite(g, spriteO, cellPx, cellPy, CELL_SIZE);
            }
        }

        // Vẽ Khung chọn (Highlight)
        if (state->status == MATCH_PLAYING) {
            DrawHighlight(hdc, state->cursor.y, state->cursor.x, CELL_SIZE, startX, startY);
        }

        // 5. Hiệu ứng Overlay Bán trong suốt khi PAUSE / KẾT THÚC
        if (state->status == MATCH_PAUSED || state->status == MATCH_FINISHED) {
            // Lớp phủ đen 70% alpha (178/255)
            Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(178, 0, 0, 0));
            g.FillRectangle(&shadowBrush, 0, 0, screenWidth, screenHeight);

            // Vẽ hộp thoại Popup
            int popupW = 500, popupH = 200;
            int popupX = (screenWidth - popupW) / 2;
            int popupY = (screenHeight - popupH) / 2;

            Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 255, 255, 255));
            Gdiplus::Pen borderPen(Gdiplus::Color(255, 100, 100, 100), 3);
            g.FillRectangle(&bgBrush, popupX, popupY, popupW, popupH);
            g.DrawRectangle(&borderPen, popupX, popupY, popupW, popupH);
        }
    } // Đối tượng Graphics bị hủy tại đây, giải phóng ngữ cảnh vẽ an toàn

    // 6. Vẽ Chữ nổi lên trên Hộp thoại Popup
    if (state->status == MATCH_PAUSED) {
        int popupY = (screenHeight - 200) / 2;
        DrawTextCentered(hdc, L"--- GAME TẠM DỪNG ---", popupY + 50, screenWidth, Colour::ORANGE_NORMAL, GlobalFont::Title);
        DrawTextCentered(hdc, L"ESC: Tiếp tục   |   Q: Về Menu", popupY + 130, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
    }
    else if (state->status == MATCH_FINISHED) {
        int popupY = (screenHeight - 200) / 2;
        std::wstring winMsg;
        COLORREF winColor;

        if (state->winner == -1) { 
            std::wstring p1NameW(state->player1.name, state->player1.name + strlen(state->player1.name));
            winMsg = p1NameW + L" CHIẾN THẮNG!"; 
            winColor = Colour::RED_NORMAL; 
        }
        else if (state->winner == 1) { 
            std::string n2 = state->player2.name;
            winMsg = std::wstring(n2.begin(), n2.end()) + L" CHIẾN THẮNG!"; 
            winColor = Colour::BLUE_NORMAL; 
        }
        else { winMsg = L"HAI BÊN HÒA NHAU!"; winColor = Colour::ORANGE_NORMAL; }

        DrawTextCentered(hdc, winMsg, popupY + 50, screenWidth, winColor, GlobalFont::Title);
        DrawTextCentered(hdc, L"Nhấn 'Y' để Chơi lại hoặc 'N' để Về Menu", popupY + 130, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
        //if (wParam == 'Y' || wParam == 'y') {
        //    // Giữ nguyên setting của ván cũ, chỉ làm mới bàn cờ
        //    InitNewMatch(state, state->gameMode, state->matchType, state->boardSize, state->countdownTime);
        //    hasChanged = true;
        //}
    }
}

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam) {
    // Bỏ qua nếu không có mã phím hợp lệ
    if (wParam == 0) return;

    // Ủy quyền xử lý cho hàm ProcessPlayInput. 
    // Nếu có sự thay đổi trạng thái (hasChanged == true), ta có thể tận dụng để
    // xử lý thêm các hiệu ứng phụ (ví dụ: phát âm thanh báo lỗi nếu bấm sai chỗ) trong tương lai.
    ProcessPlayInput(wParam, state, currentState);
}

void InitNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize, int countdownTime) {
    if (!state) return;

    // 1. Cài đặt thông số trận đấu
    state->gameMode = mode;
    state->matchType = type;
    state->boardSize = boardSize;
    state->countdownTime = countdownTime;
    state->timeRemaining = countdownTime;

    // 2. Làm sạch bàn cờ (Gán toàn bộ ô về giá trị 0 - Trống)
    for (int y = 0; y < boardSize; ++y) {
        for (int x = 0; x < boardSize; ++x) {
            state->board[y][x].c = 0;
        }
    }

    // 3. Khởi tạo thông tin người chơi (Có thể mở rộng để nhận tên từ Input sau này)
    strncpy_s(state->player1.name, "Player 1", sizeof(state->player1.name));
    state->player1.score = 0; // Đặt lại điểm số

    if (type == MATCH_PVE) {
        strncpy_s(state->player2.name, "AI (Máy)", sizeof(state->player2.name));
    }
    else {
        strncpy_s(state->player2.name, "Player 2", sizeof(state->player2.name));
    }
    state->player2.score = 0;

    // 4. Đặt lại luồng điều khiển
    state->isPlayer1Turn = true;    // X luôn đi trước
    state->status = MATCH_PLAYING;  // Bắt đầu ngay lập tức
    state->winner = 0;              // Chưa có người thắng

    // 5. Đưa con trỏ chọn ô về chính giữa bàn cờ
    state->cursor.x = boardSize / 2;
    state->cursor.y = boardSize / 2;
}