#pragma once
#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../ApplicationTypes/GameConfig.h" 
#include "../RenderAPI/Renderer.h"

const int CELL_SIZE = 40;

bool UpdatePlayLogic(PlayState* state, double dt);

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState, GameConfig* config);

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO, const GameConfig* config);

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam, GameConfig* config);