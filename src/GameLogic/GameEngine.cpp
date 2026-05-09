#include "GameEngine.h"
#include "GameRules.h"
#include "BotAI.h" // Để gọi clearTranspositionTable()
#include "PlayerEngineer.h"
#include "../RenderAPI/DirtyRect.h"
#include "../SystemModules/TimeSystem.h"
#include "../SystemModules/AudioSystem.h"

/** @file GameEngine.cpp
 *  @brief Triển khai logic quản lý ván chơi: khởi tạo match/round, undo/redo, xử lý nước đi.
 */

/**
 * @brief Khởi tạo một trận đấu mới theo tham số truyền vào và chuyển sang round đầu tiên.
 */
void initializeNewMatch(PlayState *state, PlayMode mode, MatchType type, int boardSize,
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
    state->player1TotalTimeLeft = totalTime * 60; // phút → giây
    state->player2TotalTimeLeft = totalTime * 60;

    // Khởi tạo cầu thủ qua PlayerEngineer (Nguồn sự thật duy nhất)
    initializePlayer(state->player1, state->player1.name, state->player1.avatarPath, 'X', (float)countdownTime);
    initializePlayer(state->player2, state->player2.name, state->player2.avatarPath, 'O', (float)countdownTime);

    // Xóa transposition table khi bắt đầu match mới
    clearTranspositionTable();

    startNextRound(state);
}

/**
 * @brief Bắt đầu round mới: reset bảng, trạng thái lượt, thời gian round.
 */
void startNextRound(PlayState *state)
{
    state->timeRemaining = state->countdownTime;
    state->isPlayer1Turn = true;
    state->status = MATCH_PLAYING;
    state->winner = -1;

    // Reset chỉ số lượt đánh cho round mới
    resetPlayerForRound(state->player1);
    resetPlayerForRound(state->player2);

    state->lastMoveRow = -1;
    state->lastMoveCol = -1;
    state->winningCells.clear();
    state->matchHistory.clear();
    state->redoStack.clear();
    state->matchDuration = 0.0f;

    for (int row = 0; row < MAX_BOARD_SIZE; row++)
        for (int col = 0; col < MAX_BOARD_SIZE; col++)
            state->board[row][col] = CELL_EMPTY;

    state->cursorRow = state->boardSize / 2;
    state->cursorCol = state->boardSize / 2;

    // Xóa TT khi bắt đầu round mới (bàn cờ trống = hash mới)
    clearTranspositionTable();

    resetTimer(state);
}

/**
 * @brief Chuyển lượt giữa hai người chơi và reset bộ đếm thời gian nước đi.
 */
void switchTurn(PlayState *state)
{
    state->isPlayer1Turn = !state->isPlayer1Turn;
    state->timeRemaining = state->countdownTime;
    resetTimer(state);
}

/**
 * @brief Hoàn tác nước đi gần nhất (hoặc 2 nước nếu PVE) và cập nhật trạng thái liên quan.
 */
void undoMove(PlayState *state)
{
    if (state->matchHistory.empty())
        return;

    // PVE: lùi 2 bước để trả lượt về người chơi
    int popCount = (state->matchType == MATCH_PVE && state->matchHistory.size() >= 2) ? 2 : 1;

    for (int i = 0; i < popCount; i++)
    {
        if (state->matchHistory.empty())
            break;

        auto last = state->matchHistory.back();
        state->matchHistory.pop_back();
        state->redoStack.push_back(last);

        state->board[last.first][last.second] = CELL_EMPTY;

        // Chuyển lại lượt và cập nhật movesCount đúng người
        state->isPlayer1Turn = !state->isPlayer1Turn;
        if (state->isPlayer1Turn)
            state->player1.movesCount--;
        else
            state->player2.movesCount--;
    }

    // Cập nhật lastMove
    if (state->matchHistory.empty())
    {
        state->lastMoveRow = -1;
        state->lastMoveCol = -1;
    }
    else
    {
        auto last = state->matchHistory.back();
        state->lastMoveRow = last.first;
        state->lastMoveCol = last.second;
    }

    // Mark lastMove and surrounding cell as dirty after undo/redo
    if (state->lastMoveRow >= 0 && state->lastMoveCol >= 0)
        DirtyRect::AddCell(state->lastMoveRow, state->lastMoveCol);

    // Nếu ván đã kết thúc thì reopen (trừ khi muốn giữ kết thúc)
    if (state->status == MATCH_FINISHED)
    {
        state->status = MATCH_PLAYING;
        if (state->winner == CELL_PLAYER1)
            state->player1.totalWins--;
        else if (state->winner == CELL_PLAYER2)
            state->player2.totalWins--;
        state->winner = -1;
    }
    state->winningCells.clear();
    resetTimer(state);
}

/**
 * @brief Redo các nước đã undo (1 hoặc 2 bước tuỳ PVE/PVP) và kiểm tra trạng thái thắng.
 */
void redoMove(PlayState *state)
{
    if (state->redoStack.empty())
        return;

    int popCount = (state->matchType == MATCH_PVE && state->redoStack.size() >= 2) ? 2 : 1;

    for (int i = 0; i < popCount; i++)
    {
        if (state->redoStack.empty())
            break;

        auto nextMove = state->redoStack.back();
        state->redoStack.pop_back();

        state->board[nextMove.first][nextMove.second] =
            state->isPlayer1Turn ? CELL_PLAYER1 : CELL_PLAYER2;

        state->lastMoveRow = nextMove.first;
        state->lastMoveCol = nextMove.second;
        state->matchHistory.push_back(nextMove);

        if (state->isPlayer1Turn)
            state->player1.movesCount++;
        else
            state->player2.movesCount++;

        int winStatus = checkWinCondition(state, nextMove.first, nextMove.second,
                                          &state->winningCells);
        if (winStatus != -1)
        {
            if (winStatus == CELL_PLAYER1)
                state->player1.totalWins++;
            else if (winStatus == CELL_PLAYER2)
                state->player2.totalWins++;
            state->status = MATCH_FINISHED;
            state->winner = winStatus;
        }
        else
        {
            state->isPlayer1Turn = !state->isPlayer1Turn;
        }
    }
    resetTimer(state);
}

/**
 * @brief Áp dụng một nước đi nếu hợp lệ, cập nhật lịch sử, kiểm tra thắng/thua và phát hiệu ứng âm thanh.
 * @return `true` nếu nước đi hợp lệ.
 */
bool processMove(PlayState *state, int row, int col)
{
    if (state->status != MATCH_PLAYING || !isValidMove(state, row, col))
        return false;

    int oldLastRow = state->lastMoveRow;
    int oldLastCol = state->lastMoveCol;

    state->board[row][col] = state->isPlayer1Turn ? CELL_PLAYER1 : CELL_PLAYER2;
    state->lastMoveRow = row;
    state->lastMoveCol = col;
    state->matchHistory.push_back({row, col});
    state->redoStack.clear(); // Hủy nhánh redo sau khi đặt nước mới

    // Mark dirty cells for partial invalidation (logical cells)
    DirtyRect::AddCell(row, col);
    if (oldLastRow >= 0 && oldLastCol >= 0)
        DirtyRect::AddCell(oldLastRow, oldLastCol);

    if (state->isPlayer1Turn)
        state->player1.movesCount++;
    else
        state->player2.movesCount++;

    int winStatus = checkWinCondition(state, row, col, &state->winningCells);
    if (winStatus != -1)
    {
        if (winStatus == CELL_PLAYER1)
            state->player1.totalWins++;
        else if (winStatus == CELL_PLAYER2)
            state->player2.totalWins++;

        // Kiểm tra xem đã thắng trọn series BO chưa
        int winRequired = (state->targetScore + 1) / 2; // BO1=1, BO3=2, BO5=3
        if (state->targetScore > 1)
        {
            // Dùng targetScore trực tiếp làm mục tiêu bàn thắng
            winRequired = state->targetScore;
        }
        bool isSeriesWon = (state->player1.totalWins >= winRequired || state->player2.totalWins >= winRequired);
        if (isSeriesWon)
        {
            if (state->player1.totalWins >= winRequired)
                state->player1.matchWins++;
            else
                state->player2.matchWins++;
            // Reset bàn thắng cho series BO tiếp theo (giữ nguyên matchWins)
            state->player1.totalWins = 0;
            state->player2.totalWins = 0;
        }

        // Luôn dừng ván đấu để người chơi xem kết quả

        state->status = MATCH_FINISHED;
        state->winner = winStatus;

        playSfx("sfx_whistle");
        playSfx("sfx_crowd");
        playSfx("sfx_siu");
    }
    else
    {
        switchTurn(state);
    }

    return true;
}