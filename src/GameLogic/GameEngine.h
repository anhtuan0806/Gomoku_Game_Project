#ifndef _GAME_ENGINE_H_
#define _GAME_ENGINE_H_

#include "../ApplicationTypes/PlayState.h"

// Khởi tạo ván đấu mới
// Cập nhật tham số truyền vào
void initNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize,
    int countdownTime, int difficulty, int targetScore, int totalTime = 0);

// Thêm hàm reset ván mới (giữ nguyên điểm số, chỉ reset bàn cờ cho Bo3/Bo5)
void startNextRound(PlayState* state);

// Thực hiện nước đi và cập nhật trạng thái ván đấu
bool processMove(PlayState* state, int row, int col);

// Chuyển lượt chơi
void switchTurn(PlayState* state);

void undoMove(PlayState* state);
void redoMove(PlayState* state);

#endif 