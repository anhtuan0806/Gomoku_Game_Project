#pragma once
#include "../ApplicationTypes/GameState.h"

// Xử lý phím bấm (W, S, Lên, Xuống, Enter) của người chơi bằng Raylib
void UpdateMenuScreen(ScreenState& currentState, int& selectedOption);

// Vẽ màn hình Menu với độ phân giải pixel
void RenderMenuScreen(int selectedOption, int screenWidth, int screenHeight);