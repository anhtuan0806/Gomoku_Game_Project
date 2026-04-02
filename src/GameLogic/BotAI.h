#ifndef _BOT_AI_H_
#define _BOT_AI_H_

#include "../ApplicationTypes/PlayState.h"

// Tính toán tọa độ tốt nhất cho máy (Bot luôn là PLAYER2)
// difficulty: 1 (Dễ - Random), 2 (Trung bình), 3 (Khó - AlphaBeta)
void calculateAIMove(const PlayState* state, int difficulty, int& outRow, int& outCol);

#endif // _BOT_AI_H_