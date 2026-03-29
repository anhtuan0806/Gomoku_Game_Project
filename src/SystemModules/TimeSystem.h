#pragma once
#include "../ApplicationTypes/PlayState.h"

// Trả về true nếu thời gian đếm ngược chạm 0 (hết lượt)
bool UpdateCountdown(PlayState* state, double dt);

// Đặt lại bộ tích lũy thời gian về 0 (Dùng khi bắt đầu lượt mới hoặc ván mới)
void ResetTimer();