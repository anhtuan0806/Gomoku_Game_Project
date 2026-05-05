#ifndef _TIME_SYSTEM_H_
#define _TIME_SYSTEM_H_
#include <windows.h>

#include "../ApplicationTypes/PlayState.h"

/** @file TimeSystem.h
 *  @brief Hệ thống thời gian: quản lý countdown lượt, hiển thị thời gian và các biến đo thời gian.
 */

/** @brief Cập nhật bộ đếm thời gian của `state` theo `dt` (giây).
 *  @return true nếu thời gian đếm ngược chạm 0 (hết lượt);
 */
bool updateCountdown(PlayState *state, double dt);

/** @brief Đặt lại timer của `state` về giá trị ban đầu. */
void resetTimer(PlayState *state);

/** @brief Trả về chuỗi hiển thị thời gian còn lại dạng "MM:SS" cho `state`. */
std::string getTimeDisplay(PlayState *state);

// --- Globals (Owned by TimeSystem) ---
/** @brief HANDLE dùng cho cơ chế sleep có độ chính xác cao (owned by TimeSystem). */
extern HANDLE g_FrameTimer;
/** @brief Thời điểm render trước đó (ms). */
extern double g_LastRenderMs;
/** @brief Thời điểm update trước đó (ms). */
extern double g_LastUpdateMs;
/** @brief Thời điểm blit trước đó (ms). */
extern double g_LastBlitMs;
/** @brief Thời điểm sleep trước đó (ms). */
extern double g_LastSleepMs;

#endif
