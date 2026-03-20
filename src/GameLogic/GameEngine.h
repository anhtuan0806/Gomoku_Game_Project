#pragma once
#include "../ApplicationTypes/PlayState.h"

// Khởi tạo ván đấu mới
void initNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize, int countdownTime);

// Thực hiện nước đi và cập nhật trạng thái ván đấu
bool processMove(PlayState* state, int row, int col);

// Chuyển lượt chơi
void switchTurn(PlayState* state);