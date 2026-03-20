#pragma once
#include "../ApplicationTypes/PlayState.h"

// Trả về true nếu thời gian đếm ngược chạm 0 (hết lượt)
bool UpdateCountdown(PlayState* state, double dt);