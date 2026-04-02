#include "GameEngine.h"
#include "GameRules.h"
#include "../SystemModules/TimeSystem.h"

<<<<<<< HEAD
void initNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize,
    int countdownTime, int difficulty, int targetScore, int totalTime) {
=======
void initNewMatch(PlayState* state, PlayMode mode, MatchType type,
    int boardSize, int countdownTime)
{
>>>>>>> logic-game
    state->gameMode = mode;
    state->matchType = type;
    state->boardSize = boardSize;
    state->countdownTime = countdownTime;
    state->difficulty = difficulty;
    state->targetScore = targetScore;

    // Khởi tạo thời gian tổng
    state->isMatchTimed = (totalTime > 0);
    state->p1TotalTimeLeft = totalTime * 60; // Đổi từ phút sang giây
    state->p2TotalTimeLeft = totalTime * 60;

    // Reset điểm số về 0 cho trận đấu mới
    state->p1.score = 0;
    state->p2.score = 0;

    // Gọi hàm khởi tạo bàn cờ
    startNextRound(state);
}

void startNextRound(PlayState* state) {
    state->timeRemaining = state->countdownTime;
    state->isP1Turn = true;
    state->status = MATCH_PLAYING;
    state->winner = -1;

    state->p1.movesCount = 0;
    state->p2.movesCount = 0;

    for (int r = 0; r < MAX_BOARD_SIZE; r++)
    {
        for (int c = 0; c < MAX_BOARD_SIZE; c++)
        {
            state->board[r][c] = CELL_EMPTY;
        }
    }

    state->cursorRow = state->boardSize / 2;
    state->cursorCol = state->boardSize / 2;
    ResetTimer();
}

void switchTurn(PlayState* state)
{
    state->isP1Turn = !state->isP1Turn;
    state->timeRemaining = state->countdownTime;
	ResetTimer();
}

bool processMove(PlayState* state, int row, int col)
{
    if (state->status != MATCH_PLAYING || !isValidMove(state, row, col))
    {
        return false;
    }

    // Đặt quân cờ tương ứng với lượt hiện tại
    state->board[row][col] = state->isP1Turn ? CELL_PLAYER1 : CELL_PLAYER2;

    if (state->isP1Turn) state->p1.movesCount++;
    else state->p2.movesCount++;

    int winStatus = checkWinCondition(state);
<<<<<<< HEAD
    if (winStatus != -1) { // Có kết quả thắng/thua hoặc hòa
        if (winStatus == CELL_PLAYER1) state->p1.score++;
        else if (winStatus == CELL_PLAYER2) state->p2.score++;

        // Kiểm tra xem đã đủ điểm thắng cả trận (Bo) chưa
        if (state->p1.score >= state->targetScore || state->p2.score >= state->targetScore) {
            state->status = MATCH_FINISHED;
            state->winner = winStatus;
        }
        else {
            // Nếu chưa đủ điểm thắng cả trận, reset bàn cờ để đánh ván tiếp theo
            // Hàm này đã có sẵn logic đặt state->isP1Turn = true
            startNextRound(state);
        }
    }
    else {
        // Nếu ván đấu vẫn đang tiếp tục (winStatus == -1), thực hiện chuyển lượt
=======
    if (winStatus != -1)
    {
        state->status = MATCH_FINISHED;
        state->winner = winStatus;
    }
    else
    {
>>>>>>> logic-game
        switchTurn(state);
    }

    return true;
}