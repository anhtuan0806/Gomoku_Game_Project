#include "StateUpdater.h"

void InitNewMatch(PlayState* state, PlayMode mode, MatchType type, uint8_t boardSize, uint8_t countdown) {
    state->gameMode = mode;
    state->matchType = type;
    state->boardSize = boardSize;
    state->countdownTime = countdown;
    state->timeRemaining = countdown;

    state->isPlayer1Turn = true; // P1 luôn đi trước
    state->status = MATCH_PLAYING;
    state->winner = 2; // Chưa có người thắng

    state->player1.movesCount = 0;
    state->player2.movesCount = 0;

    // Thiết lập lại dữ liệu bàn cờ bằng giá trị 0 
    for (uint8_t y = 0; y < boardSize; y++) {
        for (uint8_t x = 0; x < boardSize; x++) {
            state->board[y][x].x = x;
            state->board[y][x].y = y;
            state->board[y][x].c = 0; // 0 nghĩa là chưa ai đánh 
        }
    }

    // Thiết lập lại tọa độ hiện hành ban đầu 
    state->cursor.x = boardSize / 2;
    state->cursor.y = boardSize / 2;
}

void SwitchTurn(PlayState* state) {
    state->isPlayer1Turn = !state->isPlayer1Turn;
    state->timeRemaining = state->countdownTime; // Reset đếm ngược
}