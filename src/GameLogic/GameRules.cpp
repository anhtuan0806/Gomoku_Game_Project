#include "GameRules.h"

bool isValidMove(const PlayState* state, int row, int col) {
    if (row < 0 || row >= state->boardSize || col < 0 || col >= state->boardSize) {
        return false;
    }
    return state->board[row][col] == CELL_EMPTY;
}

int checkWinCondition(const PlayState* state) {
    int winLength = (state->gameMode == MODE_CARO) ? 5 : 3;
    int size = state->boardSize;

    // Các hướng: Ngang, Dọc, Chéo chính, Chéo phụ
    int dr[] = { 0, 1, 1, 1 };
    int dc[] = { 1, 0, 1, -1 };

    bool isBoardFull = true;

    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            if (state->board[r][c] == CELL_EMPTY) {
                isBoardFull = false;
                continue;
            }

            int currentPiece = state->board[r][c];

            for (int dir = 0; dir < 4; dir++) {
                int count = 1;
                for (int step = 1; step < winLength; step++) {
                    int nr = r + dr[dir] * step;
                    int nc = c + dc[dir] * step;

                    if (nr >= 0 && nr < size && nc >= 0 && nc < size && state->board[nr][nc] == currentPiece) {
                        count++;
                    }
                    else {
                        break;
                    }
                }
                if (count == winLength) {
                    return currentPiece; // Người chơi 1 hoặc 2 thắng
                }
            }
        }
    }

    if (isBoardFull) return CELL_EMPTY; // Hòa
    return -1; // Đang chơi
}