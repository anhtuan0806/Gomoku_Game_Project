#pragma once
#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../ApplicationTypes/GameConfig.h" 
#include "../RenderAPI/Renderer.h"

const int CELL_SIZE = 40;

enum PauseSubMenu { 
	SUB_MAIN, 
	SUB_SAVE_SELECT,
	SUB_SAVE_NAME_ENTRY
};

static int g_PauseSelected = 0;
const int TOTAL_PAUSE_ITEMS = 6;
static PauseSubMenu g_CurrentSubMenu = SUB_MAIN;
static int g_SaveSlotSelected = 0;
static std::wstring g_SaveNameInput = L""; 

bool UpdatePlayLogic(PlayState* state, double dt);

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState, GameConfig* config);

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO, const GameConfig* config);

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam, GameConfig* config);

void ResetPlayScreenStatics();