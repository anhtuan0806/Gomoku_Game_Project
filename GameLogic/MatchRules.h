#pragma once
#include "../ApplicationTypes/PlayState.h"

// Kiểm tra xem ô (x, y) trên ma trận có thể đánh được không
bool IsValidMove(const PlayState* state, uint8_t x, uint8_t y);

// Cập nhật ma trận khi người chơi đánh dấu 
bool ProcessMove(PlayState* state, uint8_t x, uint8_t y);

// Kiểm tra trạng thái ván đấu: trả về 0 (Hòa), -1 (P1 thắng), 1 (P2 thắng), 2 (Chưa kết thúc)
uint8_t CheckWinCondition(const PlayState* state);