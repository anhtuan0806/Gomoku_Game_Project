#include "GameEngine.h"
#include "GameRules.h"

void initNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize, int countdownTime) {
    state->gameMode = mode;
    state->matchType = type;
    state->boardSize = boardSize;
    state->countdownTime = countdownTime;
    state->timeRemaining = countdownTime;

    state->isP1Turn = true;
    state->status = MATCH_PLAYING;
    state->winner = -1; // Chưa kết thúc

    state->p1.movesCount = 0;
    state->p2.movesCount = 0;

    for (int r = 0; r < MAX_BOARD_SIZE; r++) {
        for (int c = 0; c < MAX_BOARD_SIZE; c++) {
            state->board[r][c] = CELL_EMPTY;
        }
    }

    state->cursorRow = boardSize / 2;
    state->cursorCol = boardSize / 2;
}

void switchTurn(PlayState* state) {
    state->isP1Turn = !state->isP1Turn;
    state->timeRemaining = state->countdownTime;
}

bool processMove(PlayState* state, int row, int col) {
    if (state->status != MATCH_PLAYING || !isValidMove(state, row, col)) {
        return false;
    }

    // Đánh cờ
    state->board[row][col] = state->isP1Turn ? CELL_PLAYER1 : CELL_PLAYER2;

    if (state->isP1Turn) state->p1.movesCount++;
    else state->p2.movesCount++;

    // Kiểm tra kết quả
    int winStatus = checkWinCondition(state);
    if (winStatus != -1) {
        state->status = MATCH_FINISHED;
        state->winner = winStatus;
    }
    else {
        switchTurn(state);
    }

    return true;
}