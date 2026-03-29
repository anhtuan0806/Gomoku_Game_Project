#include "TimeSystem.h"

static double timeAccumulator = 0.0;

bool UpdateCountdown(PlayState* state, double dt) {
    timeAccumulator += dt;

    if (timeAccumulator >= 1.0) {
        timeAccumulator -= 1.0;

        if (state->timeRemaining > 0) {
            state->timeRemaining--;
            return true; 
        }
    }
    return false;
}

// Bổ sung hàm Reset để dùng khi chuyển lượt, tránh cộng dồn thời gian thừa
void ResetTimer() {
    timeAccumulator = 0.0;
}