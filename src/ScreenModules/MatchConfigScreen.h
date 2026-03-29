#ifndef MATCH_CONFIG_SCREEN_H
#define MATCH_CONFIG_SCREEN_H

#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"

void UpdateMatchConfigScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, WPARAM wParam);
void RenderMatchConfigScreen(HDC hdc, int selectedOption, const PlayState* config, int screenWidth, int screenHeight);

#endif