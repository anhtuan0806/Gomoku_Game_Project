#include "TimeSystem.h"
#include <iomanip>
#include <sstream>

static double timeAccumulator = 0.0;

bool UpdateCountdown(PlayState* state, double dt)
{
    if (state->timeRemaining <= 0)
        return true;

    timeAccumulator += dt;

<<<<<<< HEAD
    if (timeAccumulator >= 1.0) {
=======
    // Nếu tích lũy đủ 1 giây  
    if (timeAccumulator >= 1.0)
    {
>>>>>>> logic-game
        timeAccumulator -= 1.0;
        state->timeRemaining--;

<<<<<<< HEAD
        if (state->timeRemaining > 0) {
            state->timeRemaining--;
            return true; 
=======
        if (state->timeRemaining <= 0)
        {
            state->timeRemaining = 0;
            return true;
>>>>>>> logic-game
        }
    }
    return false;
}

<<<<<<< HEAD
// Bổ sung hàm Reset để dùng khi chuyển lượt, tránh cộng dồn thời gian thừa
void ResetTimer() {
    timeAccumulator = 0.0;
}
=======
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
>>>>>>> logic-game
