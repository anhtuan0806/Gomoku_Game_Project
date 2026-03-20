#include "TimeSystem.h"

// Biến tĩnh dùng để tích lũy thời gian lẻ (miligiây) thành từng giây
static double timeAccumulator = 0.0;

bool UpdateCountdown(PlayState* state, double dt) {
    timeAccumulator += dt;

    // Nếu tích lũy đủ 1 giây (1.0)
    if (timeAccumulator >= 1.0) {
        timeAccumulator -= 1.0;

        if (state->timeRemaining > 0) {
            state->timeRemaining--;
        }

        if (state->timeRemaining <= 0) {
            return true; // Báo hiệu đã hết giờ
        }
    }
    return false;
}