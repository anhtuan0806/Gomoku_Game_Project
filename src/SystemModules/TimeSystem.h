#ifndef _TIME_SYSTEM_H_
#define _TIME_SYSTEM_H_

#include "../ApplicationTypes/PlayState.h"

// Trả về true nếu thời gian đếm ngược chạm 0 (hết lượt)
bool UpdateCountdown(PlayState* state, double dt);

<<<<<<< HEAD
// Đặt lại bộ tích lũy thời gian về 0 (Dùng khi bắt đầu lượt mới hoặc ván mới)
void ResetTimer();
=======
// Reset bộ đếm thời gian về giá trị ban đầu
void ResetTimer(PlayState* state);

// Lấy thời gian còn lại dưới dạng chuỗi "MM:SS"
std::string GetTimeDisplay(PlayState* state);

#endif
>>>>>>> logic-game
