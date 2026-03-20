#ifndef SETTING_SCREEN_H
#define SETTING_SCREEN_H

#include <windows.h>
#include "../SystemModules/ConfigLoader.h"
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../ApplicationTypes/GameConfig.h"


// Tách biệt logic xử lý thay đổi cài đặt
void ProcessSettingInput(ScreenState& currentState, GameConfig* config, int selectedOption, int direction, bool isEnterPressed);

// Hàm cập nhật trạng thái chung
void UpdateSettingScreen(ScreenState& currentState, GameConfig* config, int& selectedOption, WPARAM keyCode);

// Hàm vẽ giao diện
void RenderSettingScreen(HDC hdc, const GameConfig* config, int selectedOption, int screenWidth, int screenHeight);

#endif // SETTING_SCREEN_H