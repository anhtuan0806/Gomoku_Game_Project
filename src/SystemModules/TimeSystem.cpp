#include "TimeSystem.h"
#include <iomanip>
#include <sstream>

/** @file TimeSystem.cpp
 *  @brief Triển khai chức năng quản lý countdown lượt, format hiển thị thời gian.
 */

// --- Globals ---
HANDLE g_FrameTimer = NULL;
double g_LastRenderMs = 0.0;
double g_LastUpdateMs = 0.0;
double g_LastBlitMs = 0.0;
double g_LastSleepMs = 0.0;

static double sTimeAccumulator = 0.0;

/** @brief Cập nhật bộ đếm thời gian của `state` theo `dt` (giây).
 *  @return true nếu thời gian đã giảm ít nhất 1 giây hoặc đã hết.
 */
bool updateCountdown(PlayState *state, double dt)
{
    if (state->timeRemaining <= 0)
    {
        return true;
    }

    sTimeAccumulator += dt;

    // Nếu tích lũy đủ 1 giây
    if (sTimeAccumulator >= 1.0)
    {
        sTimeAccumulator -= 1.0;
        state->timeRemaining--;

        if (state->timeRemaining <= 0)
        {
            state->timeRemaining = 0;
        }
        return true;
    }
    return false;
}

/** @brief Đặt lại timer cho `state` về `maxTurnTime` của người chơi hiện tại.
 *  - Bảo vệ khi giá trị không hợp lệ bằng cách dùng fallback (30s hoặc `countdownTime`).
 */
void resetTimer(PlayState *state)
{
    // Áp dụng: Ép thời gian đếm ngược bằng chính thời gian của Người chơi hiện tại!
    // Tuỳ vào tới lượt ai (isPlayer1Turn), lấy maxTurnTime tương ứng.
    float maxFloatTime = state->isPlayer1Turn ? state->player1.maxTurnTime : state->player2.maxTurnTime;

    // Failsafe: Tránh lỗi load game cũ hoặc uninitialized memory khiến maxFloatTime <= 0
    if (maxFloatTime <= 0.0f)
    {
        maxFloatTime = (state->countdownTime > 0) ? static_cast<float>(state->countdownTime) : 30.0f;
        // Tự động gán lại nếu bộ nhớ trước đó chứa rác
        if (state->isPlayer1Turn)
        {
            state->player1.maxTurnTime = maxFloatTime;
        }
        else
        {
            state->player2.maxTurnTime = maxFloatTime;
        }
    }

    state->timeRemaining = static_cast<int>(maxFloatTime);
    state->countdownTime = state->timeRemaining; // Cập nhật lại thời gian gốc nếu UI cần dùng chia tỉ lệ
    sTimeAccumulator = 0.0;
}

/** @brief Trả về chuỗi hiển thị thời gian dạng "MM:SS" từ `state->timeRemaining`. */
std::string getTimeDisplay(PlayState *state)
{
    int minutes = state->timeRemaining / 60;
    int seconds = state->timeRemaining % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;

    return oss.str();
}
