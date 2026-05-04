#ifndef _TIME_SYSTEM_H_
#define _TIME_SYSTEM_H_

#include "../ApplicationTypes/PlayState.h"

// Trả về true nếu thời gian đếm ngược chạm 0 (hết lượt)
bool updateCountdown(PlayState *state, double dt);

// Reset bộ đếm thời gian về giá trị ban đầu
void resetTimer(PlayState *state);

// Lấy thời gian còn lại dưới dạng chuỗi "MM:SS"
std::string getTimeDisplay(PlayState *state);

#endif
