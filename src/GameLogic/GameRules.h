#pragma once
#include "../ApplicationTypes/PlayState.h"

// Kiểm tra xem ô cờ có đánh được không
bool isValidMove(const PlayState* state, int row, int col);

// Kiểm tra trạng thái trận đấu. 
// Trả về: CELL_PLAYER1 (1 thắng), CELL_PLAYER2 (2 thắng), CELL_EMPTY (Hòa), hoặc -1 (Chưa kết thúc)
int checkWinCondition(const PlayState* state);