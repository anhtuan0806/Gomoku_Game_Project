#ifndef _GAME_ENGINE_H_
#define _GAME_ENGINE_H_

#include "../ApplicationTypes/PlayState.h"

// Khởi tạo ván đấu mới (reset điểm, bàn cờ, timer)
void initializeNewMatch(PlayState *state, PlayMode mode, MatchType type, int boardSize,
                        int countdownTime, int difficulty, int targetScore, int totalTime = 0);

// Reset bàn cờ cho round mới (giữ nguyên điểm số, dùng trong Bo3/Bo5)
void startNextRound(PlayState *state);

// Đặt quân, cập nhật trạng thái, chuyển lượt.
// Trả về false nếu nước đi không hợp lệ.
bool processMove(PlayState *state, int row, int col);

// Chuyển lượt và reset timer
void switchTurn(PlayState *state);

// Undo / Redo (PVE: lùi/tiến 2 nước để giữ lượt người chơi)
void undoMove(PlayState *state);
void redoMove(PlayState *state);

#endif // _GAME_ENGINE_H_