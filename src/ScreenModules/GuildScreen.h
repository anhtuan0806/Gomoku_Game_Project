#ifndef GUILD_SCREEN_H
#define GUILD_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/**
 * @brief Cập nhật logic màn hình Hướng dẫn (xử lý chuyển trang và ESC).
 */
void UpdateGuildScreen(ScreenState& currentState, int& currentPage, WPARAM wParam);

/**
 * @brief Vẽ giao diện màn hình Hướng dẫn theo trang hiện tại.
 */
void RenderGuildScreen(HDC hdc, int screenWidth, int screenHeight, int currentPage);

#endif // GUILD_SCREEN_H
