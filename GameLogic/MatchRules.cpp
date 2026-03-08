#include "MatchRules.h"

bool IsValidMove(const PlayState* state, uint8_t x, uint8_t y) {
    if (x < 0 || x >= state->boardSize || y < 0 || y >= state->boardSize) 
        return false;
    // 0 nghĩa là chưa ai đánh dấu 
    return state->board[y][x].c == 0;
}

bool ProcessMove(PlayState* state, uint8_t x, uint8_t y) {
    if (IsValidMove(state, x, y)) {
        // -1 là lượt P1, 1 là lượt P2 
        state->board[y][x].c = state->isPlayer1Turn ? -1 : 1;
        state->player1.movesCount += state->isPlayer1Turn ? 1 : 0;
        state->player2.movesCount += !state->isPlayer1Turn ? 1 : 0;
        return true;
    }
    return false;
}

uint8_t CheckWinCondition(const PlayState* state) {
    uint8_t winLength = (state->gameMode == MODE_CARO) ? 5 : 3;
    uint8_t size = state->boardSize;

    // Các hướng duyệt: Ngang, Dọc, Chéo chính, Chéo phụ
    int8_t dx[] = { 1, 0, 1, 1 };
    int8_t dy[] = { 0, 1, 1, -1 };

    bool isBoardFull = true;

    for (uint8_t y = 0; y < size; y++) {
        for (uint8_t x = 0; x < size; x++) {
            if (state->board[y][x].c == 0) {
                isBoardFull = false;
                continue;
            }

            int8_t currentC = state->board[y][x].c;

            // Kiểm tra 4 hướng
            for (uint8_t dir = 0; dir < 4; dir++) {
                uint8_t count = 1;
                for (uint8_t step = 1; step < winLength; step++) {
                    uint8_t nx = x + dx[dir] * step;
                    uint8_t ny = y + dy[dir] * step;

                    if (nx >= 0 && nx < size && ny >= 0 && ny < size && state->board[ny][nx].c == currentC) {
                        count++;
                    }
                    else {
                        break;
                    }
                }
                if (count == winLength) {
                    return currentC; // Trả về -1 (P1) hoặc 1 (P2)
                }
            }
        }
    }

    if (isBoardFull) return 0; // Trả về 0 nếu hòa (ma trận đầy) 
    return 2; // 2 nghĩa là chưa ai thắng 
}