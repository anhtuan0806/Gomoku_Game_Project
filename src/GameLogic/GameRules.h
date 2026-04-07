#ifndef _GAME_RULES_H_
#define _GAME_RULES_H_

#include <vector>
#include <utility>
#include "../ApplicationTypes/PlayState.h"

// Kiểm tra xem ô cờ có đánh được không
bool isValidMove(const PlayState* state, int row, int col);

// Kiểm tra trạng thái trận đấu. 
// Kiểm tra trạng thái trận đấu. Nếu ai thắng, thêm danh sách tọa độ vào outWinCells nếu con trỏ khác nullptr.
// Trả về: CELL_PLAYER1 (1 thắng), CELL_PLAYER2 (2 thắng), CELL_EMPTY (Hòa), hoặc -1 (Chưa kết thúc)
int checkWinCondition(const PlayState* state, int lastRow, int lastCol, std::vector<std::pair<int, int>>* outWinCells = nullptr);

#endif // _GAME_RULES_H_