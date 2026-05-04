#include "GameRules.h"

bool isValidMove(const PlayState *state, int row, int col)
{
    if (row < 0 || row >= state->boardSize || col < 0 || col >= state->boardSize)
        return false;
    return state->board[row][col] == CELL_EMPTY;
}

// ============================================================
//  checkWinCondition
//      collect TẤT CẢ ô trong chuỗi thắng (không giới hạn winLength phần tử).
//       Chuỗi có thể dài hơn winLength (vd đặt ô giữa nối 2 đoạn),
//       tất cả ô đều được highlight.
// ============================================================
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
    static const int dr[] = {0, 1, 1, 1};
    static const int dc[] = {1, 0, 1, -1};

    for (int dir = 0; dir < 4; dir++)
    {
        std::vector<std::pair<int, int>> line;
        line.push_back({lastRow, lastCol});

        // Quét tiến
        for (int step = 1; step < size; step++)
        {
            int nr = lastRow + dr[dir] * step;
            int nc = lastCol + dc[dir] * step;
            if (nr >= 0 && nr < size && nc >= 0 && nc < size && state->board[nr][nc] == currentPiece)
                line.push_back({nr, nc});
            else
                break;
        }
        // Quét lùi
        for (int step = 1; step < size; step++)
        {
            int nr = lastRow - dr[dir] * step;
            int nc = lastCol - dc[dir] * step;
            if (nr >= 0 && nr < size && nc >= 0 && nc < size && state->board[nr][nc] == currentPiece)
                line.push_back({nr, nc});
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