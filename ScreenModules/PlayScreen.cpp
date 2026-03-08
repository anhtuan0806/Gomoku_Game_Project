#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../GameLogic/MatchRules.h"
#include "../GameLogic/StateUpdater.h"
#include "../GameLogic/AI_PvE.h"
#include "../SystemModules/TimeSystem.h"
#include "../SystemModules/AudioSystem.h"
#include <raylib.h>
#include <string>

// Kích thước của mỗi ô vuông trên bàn cờ (tính bằng pixel)
const int CELL_SIZE = 40;

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, double dt) {
    // 1. Cập nhật đếm ngược nếu đang chơi
    if (state->status == MATCH_PLAYING) {
        if (UpdateCountdown(state, dt)) {
            SwitchTurn(state); // Hết giờ thì mất lượt
        }
    }

    // 2. Lượt của AI (Máy)
    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isPlayer1Turn) {
        uint8_t aiX, aiY;
        CalculateAIMove(state, aiX, aiY);

        if (ProcessMove(state, aiX, aiY)) {
            PlaySFX("Asset/audio/place_piece.wav");
            int winStatus = CheckWinCondition(state);
            if (winStatus != 2) {
                state->status = MATCH_FINISHED;
                state->winner = winStatus;
                if (winStatus == 1) state->player2.score++; // Máy thắng
            }
            else {
                SwitchTurn(state);
            }
        }
        return;
    }

    // 3. Xử lý phím bấm người chơi bằng Raylib

    // NẾU ĐANG TẠM DỪNG (PAUSE)
    if (state->status == MATCH_PAUSED) {
        if (IsKeyPressed(KEY_ESCAPE)) state->status = MATCH_PLAYING; // Trở lại
        if (IsKeyPressed(KEY_Q)) currentState = SCREEN_MENU;         // Thoát ra Menu
        return;
    }

    // NẾU ĐÃ KẾT THÚC (GAME OVER)
    if (state->status == MATCH_FINISHED) {
        if (IsKeyPressed(KEY_Y)) {
            // Chơi lại ván mới 
            InitNewMatch(state, state->gameMode, state->matchType, state->boardSize, state->countdownTime);
        }
        else if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE)) {
            currentState = SCREEN_MENU; // Thoát 
        }
        return;
    }

    // NẾU ĐANG CHƠI (W, A, S, D di chuyển) 
    if (state->status == MATCH_PLAYING) {
        if (IsKeyPressed(KEY_W) && state->cursor.y > 0) state->cursor.y--;
        if (IsKeyPressed(KEY_S) && state->cursor.y < state->boardSize - 1) state->cursor.y++;
        if (IsKeyPressed(KEY_A) && state->cursor.x > 0) state->cursor.x--;
        if (IsKeyPressed(KEY_D) && state->cursor.x < state->boardSize - 1) state->cursor.x++;

        // Nhấn Enter để đánh cờ 
        if (IsKeyPressed(KEY_ENTER)) {
            if (ProcessMove(state, state->cursor.x, state->cursor.y)) {
                PlaySFX("Asset/audio/place_piece.wav");

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
        }

        // Phím Esc để tạm dừng
        if (IsKeyPressed(KEY_ESCAPE)) {
            state->status = MATCH_PAUSED;
        }
    }
}

void RenderPlayScreen(const PlayState* state, int screenWidth, int screenHeight) {
    // --- 1. TÍNH TOÁN TỌA ĐỘ BÀN CỜ ĐỂ CĂN GIỮA ---
    int boardPixelSize = state->boardSize * CELL_SIZE;
    int startX = (screenWidth - boardPixelSize) / 2;
    int startY = (screenHeight - boardPixelSize) / 2 + 30; // Đẩy lùi xuống 30px nhường chỗ cho tiêu đề

    // --- 2. VẼ UI THÔNG TIN BÊN TRÊN ---
    std::string modeTitle = (state->gameMode == MODE_CARO) ? "--- CARO ---" : "--- TIC-TAC-TOE ---";
    DrawTextCentered(20, screenWidth, modeTitle.c_str(), 30, DARKBLUE);

    // Điểm số Player 1 (Bên trái)
    std::string p1Text = std::string(state->player1.name) + " (X): " + std::to_string(state->player1.score);
    DrawTextNormal(50, 60, p1Text.c_str(), 20, RED);

    // Điểm số Player 2 (Bên phải)
    std::string p2Text = std::string(state->player2.name) + " (O): " + std::to_string(state->player2.score);
    int p2Width = MeasureText(p2Text.c_str(), 20);
    DrawTextNormal(screenWidth - p2Width - 50, 60, p2Text.c_str(), 20, BLUE);

    // Lượt đi & Thời gian (Ở giữa)
    std::string turnText = state->isPlayer1Turn ? "Luot cua: " + std::string(state->player1.name) : "Luot cua: " + std::string(state->player2.name);
    Color turnColor = state->isPlayer1Turn ? RED : BLUE;
    DrawTextCentered(60, screenWidth, turnText.c_str(), 25, turnColor);

    std::string timeText = "Thoi gian: " + std::to_string(state->timeRemaining) + "s";
    DrawTextCentered(90, screenWidth, timeText.c_str(), 20, DARKGRAY);

    // --- 3. VẼ BÀN CỜ VÀ QUÂN CỜ ---
    DrawGridBoard(startX, startY, state->boardSize, CELL_SIZE, LIGHTGRAY);

    for (int y = 0; y < state->boardSize; y++) {
        for (int x = 0; x < state->boardSize; x++) {
            if (state->board[y][x].c != 0) {
                // Tọa độ pixel của từng ô
                int cellPx = startX + x * CELL_SIZE;
                int cellPy = startY + y * CELL_SIZE;
                DrawPiece(cellPx, cellPy, CELL_SIZE, state->board[y][x].c);
            }
        }
    }

    // --- 4. VẼ CON TRỎ (HIGHLIGHT) ---
    if (state->status == MATCH_PLAYING) {
        int cursorPx = startX + state->cursor.x * CELL_SIZE;
        int cursorPy = startY + state->cursor.y * CELL_SIZE;
        DrawCursorHighlight(cursorPx, cursorPy, CELL_SIZE, YELLOW);
    }

    // --- 5. HIỆU ỨNG POP-UP (OVERLAY) KHI PAUSE / GAME OVER ---
    if (state->status == MATCH_PAUSED || state->status == MATCH_FINISHED) {
        // Vẽ một màn đen bán trong suốt bao phủ toàn bộ màn hình!
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));

        // Vẽ khung trắng ở giữa
        int popupWidth = 500;
        int popupHeight = 200;
        int popupX = (screenWidth - popupWidth) / 2;
        int popupY = (screenHeight - popupHeight) / 2;
        DrawRectangle(popupX, popupY, popupWidth, popupHeight, RAYWHITE);
        DrawRectangleLines(popupX, popupY, popupWidth, popupHeight, DARKGRAY);

        if (state->status == MATCH_PAUSED) {
            DrawTextCentered(popupY + 50, screenWidth, "--- GAME TAM DUNG ---", 30, ORANGE);
            DrawTextCentered(popupY + 120, screenWidth, "ESC: Tiep tuc   |   Q: Ve Menu", 20, DARKGRAY);
        }
        else if (state->status == MATCH_FINISHED) {
            std::string winMsg;
            Color winColor;
            if (state->winner == -1) {
                winMsg = std::string(state->player1.name) + " CHIEN THANG!"; // 
                winColor = RED;
            }
            else if (state->winner == 1) {
                winMsg = std::string(state->player2.name) + " CHIEN THANG!"; // 
                winColor = BLUE;
            }
            else {
                winMsg = "HAI BEN HOA NHAU!"; 
                winColor = ORANGE;
            }

            DrawTextCentered(popupY + 50, screenWidth, winMsg.c_str(), 35, winColor);
            DrawTextCentered(popupY + 120, screenWidth, "Nhan 'Y' de Choi lai hoac 'N' de Ve Menu", 20, DARKGRAY); // [cite: 17, 19]
        }
    }
}