#ifndef GUILD_SCREEN_H
#define GUILD_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/** @file GuildScreen.h
 *  @brief API cho màn Hướng dẫn/Guild (trang nội dung nhiều trang).
 */

/** @brief Cập nhật logic màn Hướng dẫn (chuyển trang, ESC về menu).
 *  @param currentState Tham chiếu trạng thái màn để có thể chuyển về Menu.
 *  @param currentPage Tham chiếu trang hiện tại (sẽ tăng/giảm khi chuyển).
 *  @param wParam Mã phím nhận từ vòng lặp chính.
 */
bool UpdateGuildScreen(ScreenState &currentState, int &currentPage, WPARAM wParam);

/** @brief Vẽ nội dung Hướng dẫn cho `currentPage`.
 *  @param hdc Device context để vẽ.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 *  @param currentPage Trang nội dung đang hiển thị.
 */
void RenderGuildScreen(HDC hdc, int screenWidth, int screenHeight, int currentPage);

#endif // GUILD_SCREEN_H
