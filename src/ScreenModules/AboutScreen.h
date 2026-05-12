#ifndef ABOUT_SCREEN_H
#define ABOUT_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/** @file AboutScreen.h
 *  @brief API cho màn giới thiệu/About.
 *
 *  Gồm hàm cập nhật (ví dụ trả về menu khi nhấn ESC) và hàm vẽ nội dung About.
 */

/** @brief Cập nhật logic màn About (xử lý phím như ESC để quay về menu).
 *  @param currentState Tham chiếu để có thể chuyển trở lại Menu.
 *  @param wParam Mã phím nhận từ vòng lặp chính.
 */
bool UpdateAboutScreen(ScreenState &currentState, WPARAM wParam);

/** @brief Vẽ giao diện About lên `hdc`.
 *  @param hdc Device context dùng để vẽ.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 */
void RenderAboutScreen(HDC hdc, int screenWidth, int screenHeight);

#endif // ABOUT_SCREEN_H
