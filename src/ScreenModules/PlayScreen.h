#ifndef _PLAY_SCREEN_H_
#define _PLAY_SCREEN_H_
#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../ApplicationTypes/GameConfig.h"
#include "../RenderAPI/Renderer.h"

enum PauseSubMenu
{
	SUB_MAIN,
	SUB_SAVE_SELECT,
	SUB_SAVE_NAME_ENTRY
};

static int g_PauseSelected = 0;
const int TOTAL_PAUSE_ITEMS = 5;
static PauseSubMenu g_CurrentSubMenu = SUB_MAIN;
static int g_SaveSlotSelected = 0;
static std::wstring g_SaveNameInput = L"";

bool UpdatePlayLogic(PlayState *state, double dt);

bool ProcessPlayInput(WPARAM wParam, PlayState *state, ScreenState &currentState, GameConfig *config);

void RenderPlayScreen(HDC hdc, const PlayState *state, int screenWidth, int screenHeight, const GameConfig *config);

void UpdatePlayScreen(PlayState *state, ScreenState &currentState, WPARAM wParam, GameConfig *config);

void ResetPlayScreenStatics();

#endif // _PLAY_SCREEN_H_