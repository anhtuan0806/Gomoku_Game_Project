#ifndef LOAD_GAME_SCREEN_H
#define LOAD_GAME_SCREEN_H
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include <string>
#include <windows.h>

/** @file LoadGameScreen.h
 *  @brief API cho màn hình quản lý save/load.
 *
 *  Chứa các hàm để xử lý input, cập nhật trạng thái và vẽ danh sách save.
 */

/** @brief Xử lý sự kiện phím cho màn hình Tải Game.
 *  @param wParam Mã phím/flags.
 *  @param currentState Tham chiếu trạng thái màn để có thể chuyển màn.
 *  @param playState Con trỏ `PlayState` để load vào khi chọn một slot.
 *  @param selectedOption Chỉ mục slot đang chọn.
 *  @param statusMessage Tham chiếu thông báo trạng thái (ví dụ "Load thành công").
 *  @return `true` nếu cần vẽ lại màn hình.
 */
bool ProcessLoadGameInput(WPARAM wParam, ScreenState &currentState, PlayState *playState, int &selectedOption, std::wstring &statusMessage);

/** @brief Vẽ danh sách save và UI liên quan lên `hdc`.
 *  @param hdc Device context dùng để vẽ.
 *  @param selectedOption Slot đang chọn.
 *  @param statusMessage Thông báo trạng thái hiển thị phía dưới.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 */
void RenderLoadGameScreen(HDC hdc, int selectedOption, const std::wstring &statusMessage, int screenWidth, int screenHeight);

/** @brief Nhận thông điệp từ `WndProc` và điều phối xử lý input/logic.
 *  @param currentState Tham chiếu trạng thái màn hình.
 *  @param playState Con trỏ `PlayState` cho thao tác load.
 *  @param selectedOption Chỉ mục slot đang chọn (tham chiếu để chỉnh sửa).
 *  @param statusMessage Tham chiếu chuỗi trạng thái cập nhật.
 *  @param wParam Mã phím/flags nhận từ vòng lặp chính.
 */
bool UpdateLoadGameScreen(ScreenState &currentState, PlayState *playState, int &selectedOption, std::wstring &statusMessage, WPARAM wParam);

#endif // LOAD_GAME_SCREEN_H