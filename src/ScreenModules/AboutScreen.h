#ifndef ABOUT_SCREEN_H
#define ABOUT_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/**
 * @brief Cập nhật logic màn hình Giới thiệu (xử lý ESC).
 */
void UpdateAboutScreen(ScreenState& currentState, WPARAM wParam);

/**
 * @brief Vẽ giao diện màn hình Giới thiệu.
 */
void RenderAboutScreen(HDC hdc, int screenWidth, int screenHeight);

#endif // ABOUT_SCREEN_H
