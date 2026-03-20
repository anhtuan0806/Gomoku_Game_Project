#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/**
 * @brief Hàm giao tiếp chính để cập nhật trạng thái Menu từ vòng lặp thông điệp.
 */
void UpdateMenuScreen(ScreenState& currentState, int& selectedOption, WPARAM wParam);

/**
 * @brief Xử lý logic phím nhấn và thay đổi lựa chọn hiện tại.
 * @return true nếu có sự thay đổi trạng thái hoặc lựa chọn, false nếu không.
 */
bool ProcessMenuInput(WPARAM wParam, ScreenState& currentState, int& selectedOption);

/**
 * @brief Vẽ giao diện màn hình Menu sử dụng GDI.
 */
void RenderMenuScreen(HDC hdc, int selectedOption, int screenWidth, int screenHeight);

#endif // MENU_SCREEN_H