#include "GameEngine.h"
#include "GameRules.h"
#include "BotAI.h"                          // Để gọi clearTranspositionTable()
#include "../SystemModules/TimeSystem.h"
#include "../SystemModules/AudioSystem.h"

// ============================================================
//  initNewMatch
// ============================================================
void initNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize,
    int countdownTime, int difficulty, int targetScore, int totalTime)
{
    state->gameMode = mode;
    state->matchType = type;
    state->boardSize = boardSize;
    state->countdownTime = countdownTime;
    state->difficulty = difficulty;
    state->targetScore = targetScore;

    // Thời gian tổng cho cả trận
    state->isMatchTimed = (totalTime > 0);
    state->p1TotalTimeLeft = totalTime * 60; // phút → giây
    state->p2TotalTimeLeft = totalTime * 60;

    // Reset điểm về 0
    state->p1.totalWins = 0;
    state->p2.totalWins = 0;
    state->p1.matchWins = 0;
    state->p2.matchWins = 0;
    state->p1.totalTimePossessed = 0.0f;
    state->p2.totalTimePossessed = 0.0f;

    // Giới hạn thời gian lượt
    state->p1.maxTurnTime = static_cast<float>(countdownTime);
    state->p2.maxTurnTime = static_cast<float>(countdownTime);

    // Xóa transposition table khi bắt đầu match mới
    clearTranspositionTable();

    startNextRound(state);
}

// ============================================================
//  startNextRound
// ============================================================
void startNextRound(PlayState* state) {
    state->timeRemaining = state->countdownTime;
    state->isP1Turn = true;
    state->status = MATCH_PLAYING;
    state->winner = -1;

    state->p1.movesCount = 0;
    state->p2.movesCount = 0;

    state->lastMoveRow = -1;
    state->lastMoveCol = -1;
    state->winningCells.clear();
    state->matchHistory.clear();
    state->redoStack.clear();
    state->matchDuration = 0.0f;

    for (int r = 0; r < MAX_BOARD_SIZE; r++)
        for (int c = 0; c < MAX_BOARD_SIZE; c++)
            state->board[r][c] = CELL_EMPTY;

    state->cursorRow = state->boardSize / 2;
    state->cursorCol = state->boardSize / 2;

    // Xóa TT khi bắt đầu round mới (bàn cờ trống = hash mới)
    clearTranspositionTable();

    ResetTimer(state);
}

// ============================================================
//  switchTurn
// ============================================================
void switchTurn(PlayState* state) {
    state->isP1Turn = !state->isP1Turn;
    state->timeRemaining = state->countdownTime;
    ResetTimer(state);
}

// ============================================================
//  undoMove
// ============================================================
void undoMove(PlayState* state) {
    if (state->matchHistory.empty()) return;

    // PVE: lùi 2 bước để trả lượt về người chơi
    int popCount = (state->matchType == MATCH_PVE
        && state->matchHistory.size() >= 2) ? 2 : 1;

    for (int i = 0; i < popCount; i++) {
        if (state->matchHistory.empty()) break;

        auto last = state->matchHistory.back();
        state->matchHistory.pop_back();
        state->redoStack.push_back(last);

        state->board[last.first][last.second] = CELL_EMPTY;

        // Chuyển lại lượt và cập nhật movesCount đúng người
        state->isP1Turn = !state->isP1Turn;
        if (state->isP1Turn)
            state->p1.movesCount--;
        else
            state->p2.movesCount--;
    }

    // Cập nhật lastMove
    if (state->matchHistory.empty()) {
        state->lastMoveRow = -1;
        state->lastMoveCol = -1;
    }
    else {
        auto last = state->matchHistory.back();
        state->lastMoveRow = last.first;
        state->lastMoveCol = last.second;
    }

    // Nếu ván đã kết thúc thì reopen
    if (state->status == MATCH_FINISHED) {
        state->status = MATCH_PLAYING;
        if (state->winner == CELL_PLAYER1)      state->p1.totalWins--;
        else if (state->winner == CELL_PLAYER2) state->p2.totalWins--;
        state->winner = -1;
    }
    state->winningCells.clear();
}

// ============================================================
//  redoMove
// ============================================================
void redoMove(PlayState* state) {
    if (state->redoStack.empty()) return;

    int popCount = (state->matchType == MATCH_PVE
        && state->redoStack.size() >= 2) ? 2 : 1;

    for (int i = 0; i < popCount; i++) {
        if (state->redoStack.empty()) break;

        auto nextMove = state->redoStack.back();
        state->redoStack.pop_back();

        state->board[nextMove.first][nextMove.second] =
            state->isP1Turn ? CELL_PLAYER1 : CELL_PLAYER2;

        state->lastMoveRow = nextMove.first;
        state->lastMoveCol = nextMove.second;
        state->matchHistory.push_back(nextMove);

        if (state->isP1Turn) state->p1.movesCount++;
        else                  state->p2.movesCount++;

        int winStatus = checkWinCondition(state, nextMove.first, nextMove.second,
            &state->winningCells);
        if (winStatus != -1) {
            if (winStatus == CELL_PLAYER1) state->p1.totalWins++;
            else if (winStatus == CELL_PLAYER2) state->p2.totalWins++;
            state->status = MATCH_FINISHED;
            state->winner = winStatus;
        }
        else {
            state->isP1Turn = !state->isP1Turn;
        }
    }
}

// ============================================================
//  processMove
// ============================================================
bool processMove(PlayState* state, int row, int col) {
    if (state->status != MATCH_PLAYING || !isValidMove(state, row, col))
        return false;

    state->board[row][col] = state->isP1Turn ? CELL_PLAYER1 : CELL_PLAYER2;
    state->lastMoveRow = row;
    state->lastMoveCol = col;
    state->matchHistory.push_back({ row, col });
    state->redoStack.clear(); // Hủy nhánh redo sau khi đặt nước mới

    if (state->isP1Turn) state->p1.movesCount++;
    else                  state->p2.movesCount++;

    int winStatus = checkWinCondition(state, row, col, &state->winningCells);
    if (winStatus != -1) {
        if (winStatus == CELL_PLAYER1)      state->p1.totalWins++;
        else if (winStatus == CELL_PLAYER2) state->p2.totalWins++;

        // Kiểm tra xem đã thắng trọn series BO chưa
        int winRequired = (state->targetScore + 1) / 2; // BO1=1, BO3=2, BO5=3
        if (state->targetScore > 1) {
            // Dùng targetScore trực tiếp làm mục tiêu bàn thắng
            winRequired = state->targetScore;
        }
        bool seriesWon = (state->p1.totalWins >= winRequired || state->p2.totalWins >= winRequired);
        if (seriesWon) {
            if (state->p1.totalWins >= winRequired) state->p1.matchWins++;
            else state->p2.matchWins++;
            // Reset bàn thắng cho series BO tiếp theo (giữ nguyên matchWins)
            state->p1.totalWins = 0;
            state->p2.totalWins = 0;
        }

        // Luôn dừng ván đấu để người chơi xem kết quả

        state->status = MATCH_FINISHED;
        state->winner = winStatus;

        PlaySFX("sfx_whistle");
        PlaySFX("sfx_crowd");

        // Bonus quả "Siuuu" thương hiệu
        PlaySFX("sfx_siu");
    }
    else {
        switchTurn(state);
    }

    return true;
}