#include "GameRules.h"

bool isValidMove(const PlayState* state, int row, int col)
{
    if (row < 0 || row >= state->boardSize || col < 0 || col >= state->boardSize)
    {
        return false;
    }
    return state->board[row][col] == CELL_EMPTY;
}

int checkWinCondition(const PlayState* state, int lastRow, int lastCol, std::vector<std::pair<int, int>>* outWinCells)
{
    int winLength = (state->gameMode == MODE_CARO) ? 5 : 3;
    int size = state->boardSize;

    if (lastRow < 0 || lastRow >= size || lastCol < 0 || lastCol >= size) {
        return -1;
    }

    int currentPiece = state->board[lastRow][lastCol];
    if (currentPiece == CELL_EMPTY) {
        return -1;
    }

    // Các hướng: Ngang, Dọc, Chéo chính, Chéo phụ (chỉ cần lấy 1 nửa phương chiều)
    int dr[] = { 0, 1, 1, 1 };
    int dc[] = { 1, 0, 1, -1 };

    for (int dir = 0; dir < 4; dir++)
    {
        int count = 1;
        std::vector<std::pair<int, int>> currentLine;
        currentLine.push_back({lastRow, lastCol});

        // Quét về phía tiến
        for (int step = 1; step < winLength; step++)
        {
            int nr = lastRow + dr[dir] * step;
            int nc = lastCol + dc[dir] * step;
            if (nr >= 0 && nr < size && nc >= 0 && nc < size && state->board[nr][nc] == currentPiece) {
                count++;
                currentLine.push_back({nr, nc});
            }
            else {
                break;
            }
        }

        // Quét về phía lùi
        for (int step = 1; step < winLength; step++)
        {
            int nr = lastRow - dr[dir] * step;
            int nc = lastCol - dc[dir] * step;
            if (nr >= 0 && nr < size && nc >= 0 && nc < size && state->board[nr][nc] == currentPiece) {
                count++;
                currentLine.push_back({nr, nc});
            }
            else {
                break;
            }
        }

        if (count >= winLength)
        {
            if (outWinCells) {
                *outWinCells = currentLine;
            }
            return currentPiece; // Người chơi 1 hoặc 2 thắng
        }
    }

    if (state->p1.movesCount + state->p2.movesCount == size * size) return CELL_EMPTY; // Hòa
    return -1; // Đang chơi
}