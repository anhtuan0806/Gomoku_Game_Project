#include "TimeSystem.h"
#include <chrono>

static std::chrono::steady_clock::time_point g_lastTime;
static double g_timeAccumulator = 0.0;

void InitTimeSystem() {
    g_lastTime = std::chrono::steady_clock::now();
    g_timeAccumulator = 0.0;
}

double GetDeltaTime() {
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - g_lastTime;
    g_lastTime = currentTime;
    return elapsed.count();
}

bool UpdateCountdown(PlayState* state, double dt) {
    if (state->status != MATCH_PLAYING) return false;

    g_timeAccumulator += dt;

    // Nếu trôi qua 1 giây
    if (g_timeAccumulator >= 1.0) {
        state->timeRemaining -= 1;
        g_timeAccumulator -= 1.0;

        if (state->timeRemaining <= 0) {
            state->timeRemaining = 0;
            return true; // Báo hiệu đã hết giờ
        }
    }
    return false;
}