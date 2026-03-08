#pragma once
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/GameConfig.h"

// Xử lý phím bấm điều hướng và thay đổi giá trị cài đặt bằng Raylib
void UpdateSettingScreen(ScreenState& currentState, GameConfig* config, int& selectedOption);

// Vẽ giao diện cài đặt với độ phân giải pixel, thanh âm lượng và các lựa chọn
void RenderSettingScreen(const GameConfig* config, int selectedOption, int screenWidth, int screenHeight);