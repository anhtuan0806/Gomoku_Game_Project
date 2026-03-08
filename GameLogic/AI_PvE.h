#pragma once
#include "../ApplicationTypes/PlayState.h"

// Tính toán và trả về tọa độ (outX, outY) tốt nhất cho Máy
void CalculateAIMove(const PlayState* state, uint8_t& outX, uint8_t& outY);