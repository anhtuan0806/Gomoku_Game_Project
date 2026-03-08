#pragma once
#include "../ApplicationTypes/PlayState.h"
#include "../ApplicationTypes/GameState.h"

// Cập nhật logic ván đấu (Nhận phím, đếm ngược, AI)
void UpdatePlayScreen(PlayState* state, ScreenState& currentState, double dt);

// Vẽ bàn cờ, quân cờ và giao diện trong trận (yêu cầu truyền thêm width/height để căn giữa)
void RenderPlayScreen(const PlayState* state, int screenWidth, int screenHeight);