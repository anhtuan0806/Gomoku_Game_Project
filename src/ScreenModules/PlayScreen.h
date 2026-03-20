#pragma once
#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../RenderAPI/Renderer.h"

const int CELL_SIZE = 40;

// Cập nhật logic tự động (Thời gian đếm ngược, AI tự động đánh)
bool UpdatePlayLogic(PlayState* state, double dt);

// Xử lý sự kiện bàn phím của người chơi
bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState);

// Kết xuất màn hình Play
void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO);

// Định tuyến Input
void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam);