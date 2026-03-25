#include "TimeSystem.h"
#include <iomanip>
#include <sstream>

// Biến tĩnh dùng để tích lũy thời gian lẻ (miligiây) thành từng giây
static double timeAccumulator = 0.0;

bool UpdateCountdown(PlayState* state, double dt)
{
    if (state->timeRemaining <= 0)
        return true;

    timeAccumulator += dt;

    // Nếu tích lũy đủ 1 giây  
    if (timeAccumulator >= 1.0)
    {
        timeAccumulator -= 1.0;
        state->timeRemaining--;

        if (state->timeRemaining <= 0)
        {
            state->timeRemaining = 0;
            return true;
        }
    }
    return false;
}

void ResetTimer(PlayState* state)
{
    // Áp dụng: Ép thời gian đếm ngược bằng chính thời gian của Người chơi hiện tại!
    // Tuỳ vào tới lượt ai (isP1Turn), lấy maxTurnTime tương ứng.
    float maxFloatTime = state->isP1Turn ? state->p1.maxTurnTime : state->p2.maxTurnTime;

    state->timeRemaining = static_cast<int>(maxFloatTime);
    state->countdownTime = state->timeRemaining; // Cập nhật lại thời gian gốc nếu UI cần dùng chia tỉ lệ
    timeAccumulator = 0.0;
}

std::string GetTimeDisplay(PlayState* state)
{
    int minutes = state->timeRemaining / 60;
    int seconds = state->timeRemaining % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;

    return oss.str();
}
