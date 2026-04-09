#ifndef GUILD_SCREEN_H
#define GUILD_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/**
 * @brief Cập nhật logic màn hình Hướng dẫn (xử lý ESC).
 */
void UpdateGuildScreen(ScreenState& currentState, WPARAM wParam);

/**
 * @brief Vẽ giao diện màn hình Hướng dẫn.
 */
void RenderGuildScreen(HDC hdc, int screenWidth, int screenHeight);

#endif // GUILD_SCREEN_H
