#ifndef _GAME_RULES_H_
#define _GAME_RULES_H_

#include <vector>
#include <utility>
#include "../ApplicationTypes/PlayState.h"

// Kiểm tra ô có hợp lệ để đánh không
bool isValidMove(const PlayState* state, int row, int col);

// Kiểm tra trạng thái trận đấu sau nước đi tại (lastRow, lastCol).
// outWinCells: nếu khác nullptr, sẽ được fill toàn bộ ô thuộc chuỗi thắng (FIX: đủ ô).
// Trả về: CELL_PLAYER1 (P1 thắng), CELL_PLAYER2 (P2 thắng),
//         CELL_EMPTY (Hòa), hoặc -1 (Chưa kết thúc)
int checkWinCondition(const PlayState* state, int lastRow, int lastCol,
    std::vector<std::pair<int, int>>* outWinCells = nullptr);

#endif // _GAME_RULES_H_