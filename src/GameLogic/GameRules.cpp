#include "GameRules.h"

/** @file GameRules.cpp
 *  @brief Cài đặt các luật chơi cơ bản: xác thực nước đi và kiểm tra điều kiện thắng/hòa.
 */

bool isValidMove(const PlayState *state, int row, int col)
{
    if (row < 0 || row >= state->boardSize || col < 0 || col >= state->boardSize)
        return false;
    return state->board[row][col] == CELL_EMPTY;
}

/**
 * @brief Kiểm tra điều kiện thắng sau khi đặt quân tại (lastRow,lastCol).
 *
 * Hàm thu thập toàn bộ ô thuộc chuỗi thắng (có thể dài hơn `winLength`).
 * @return `CELL_PLAYER1`/`CELL_PLAYER2` nếu có người thắng; `CELL_EMPTY` nếu hòa; -1 nếu ván chưa kết thúc.
 */
int checkWinCondition(const PlayState *state, int lastRow, int lastCol,
                      std::vector<std::pair<int, int>> *outWinCells)
{
    const int winLength = (state->gameMode == MODE_CARO) ? 5 : 3;
    const int size = state->boardSize;

    if (lastRow < 0 || lastRow >= size || lastCol < 0 || lastCol >= size)
        return -1;

    const int currentPiece = state->board[lastRow][lastCol];
    if (currentPiece == CELL_EMPTY)
        return -1;

    // 4 hướng: ngang, dọc, chéo chính, chéo phụ
    static const int dirRow[] = {0, 1, 1, 1};
    static const int dirCol[] = {1, 0, 1, -1};

    for (int dir = 0; dir < 4; dir++)
    {
        std::vector<std::pair<int, int>> line;
        line.push_back({lastRow, lastCol});

        // Quét tiến
        for (int step = 1; step < size; step++)
        {
            int nextRow = lastRow + dirRow[dir] * step;
            int nextCol = lastCol + dirCol[dir] * step;
            if (nextRow >= 0 && nextRow < size && nextCol >= 0 && nextCol < size && state->board[nextRow][nextCol] == currentPiece)
                line.push_back({nextRow, nextCol});
            else
                break;
        }
        // Quét lùi
        for (int step = 1; step < size; step++)
        {
            int nextRow = lastRow - dirRow[dir] * step;
            int nextCol = lastCol - dirCol[dir] * step;
            if (nextRow >= 0 && nextRow < size && nextCol >= 0 && nextCol < size && state->board[nextRow][nextCol] == currentPiece)
                line.push_back({nextRow, nextCol});
            else
                break;
        }

        if ((int)line.size() >= winLength)
        {
            if (outWinCells)
                *outWinCells = line; // Trả về toàn bộ chuỗi, không cắt
            return currentPiece;
        }
    }

    // Kiểm tra hòa: toàn bộ bàn đã đầy
    if (state->player1.movesCount + state->player2.movesCount == size * size)
        return CELL_EMPTY;

    return -1; // Ván đang tiếp tục
}