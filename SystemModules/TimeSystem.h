#pragma once
#include "../ApplicationTypes/PlayState.h"

// Khởi tạo hệ thống đếm thời gian
void InitTimeSystem();

// Lấy thời gian trôi qua giữa 2 vòng lặp (Delta Time)
double GetDeltaTime();

// Cập nhật đếm ngược. Trả về true nếu đã hết giờ trong lượt này.
bool UpdateCountdown(PlayState* state, double dt);