#include "GameEngine.h"
#include "GameRules.h"
#include "../SystemModules/TimeSystem.h"

void initNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize,
    int countdownTime, int difficulty, int targetScore, int totalTime) 
    {
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
    state->p1.totalWins = 0;
    state->p2.totalWins = 0;
    state->p1.matchWins = 0;
    state->p2.matchWins = 0;
    state->p1.totalTimePossessed = 0.0f;
    state->p2.totalTimePossessed = 0.0f;

    // Khởi tạo thời gian lượt chờ cho PlayerInfo2 (quan trọng để ResetTimer không bị 0)
    state->p1.maxTurnTime = static_cast<float>(countdownTime);
    state->p2.maxTurnTime = static_cast<float>(countdownTime);

    // Gọi hàm khởi tạo bàn cờ
    startNextRound(state);
}

void startNextRound(PlayState* state) 
{
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
    {
        for (int c = 0; c < MAX_BOARD_SIZE; c++)
        {
            state->board[r][c] = CELL_EMPTY;
        }
    }

    state->cursorRow = state->boardSize / 2;
    state->cursorCol = state->boardSize / 2;
    ResetTimer(state);
}

void switchTurn(PlayState* state)
{
    state->isP1Turn = !state->isP1Turn;
    state->timeRemaining = state->countdownTime;
    ResetTimer(state);
}

void undoMove(PlayState* state) {
    if (state->matchHistory.empty()) {
        return;
    }
    
    // Nếu là PVE, lùi 2 bước để tới lượt lại của người chơi, trừ khi lịch sử chỉ có 1 bước
    int popCount = (state->matchType == MATCH_PVE && state->matchHistory.size() >= 2) ? 2 : 1;
    
    for (int i = 0; i < popCount; i++) {
        if (state->matchHistory.empty()) {
            break;
        }
        auto last = state->matchHistory.back();
        state->matchHistory.pop_back();
        state->redoStack.push_back(last);
        
        state->board[last.first][last.second] = CELL_EMPTY;
        
        state->isP1Turn = !state->isP1Turn;
        if (state->isP1Turn) {
            state->p1.movesCount--;
        }
        else {
            state->p2.movesCount--;
        }
    }
    
    if (state->matchHistory.empty()) {
        state->lastMoveRow = -1;
        state->lastMoveCol = -1;
    } 
    else {
        auto last = state->matchHistory.back();
        state->lastMoveRow = last.first;
        state->lastMoveCol = last.second;
    }
    
    if (state->status == MATCH_FINISHED) {
        state->status = MATCH_PLAYING;
        if (state->winner == CELL_PLAYER1) {
            state->p1.totalWins--;
        }
        else if (state->winner == CELL_PLAYER2) {
            state->p2.totalWins--;
        }
        state->winner = -1;
    }
    state->winningCells.clear();
}

void redoMove(PlayState* state) {
    if (state->redoStack.empty()) {
        return;
    }
    
    int popCount = (state->matchType == MATCH_PVE && state->redoStack.size() >= 2) ? 2 : 1;

    for (int i = 0; i < popCount; i++) {
        if (state->redoStack.empty()) {
            break;
        }
        auto nextMove = state->redoStack.back();
        state->redoStack.pop_back();
        
        state->board[nextMove.first][nextMove.second] = state->isP1Turn ? CELL_PLAYER1 : CELL_PLAYER2;
        state->lastMoveRow = nextMove.first;
        state->lastMoveCol = nextMove.second;
        state->matchHistory.push_back(nextMove);

        if (state->isP1Turn) {
            state->p1.movesCount++;
        }
        else {
            state->p2.movesCount++;
        }

        int winStatus = checkWinCondition(state, nextMove.first, nextMove.second, &state->winningCells);
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

bool processMove(PlayState* state, int row, int col)
{
    if (state->status != MATCH_PLAYING || !isValidMove(state, row, col))
    {
        return false;
    }

    // Đặt quân cờ tương ứng với lượt hiện tại
    state->board[row][col] = state->isP1Turn ? CELL_PLAYER1 : CELL_PLAYER2;
    state->lastMoveRow = row;
    state->lastMoveCol = col;
    state->matchHistory.push_back({row, col});
    state->redoStack.clear(); // Hủy lịch sử rẽ nhánh tương lai

    if (state->isP1Turn) {
        state->p1.movesCount++;
    }
    else {
        state->p2.movesCount++;
    }

    int winStatus = checkWinCondition(state, row, col, &state->winningCells);
    if (winStatus != -1) { 
        if (winStatus == CELL_PLAYER1) state->p1.totalWins++;
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
    }
    else {
        // Nếu ván đấu vẫn đang tiếp tục, thực hiện chuyển lượt
        switchTurn(state);
    }

    return true;
}