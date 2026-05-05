#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include <windows.h>
#include "../ApplicationTypes/GameState.h"

/** @file MenuScreen.h
 *  @brief Khai báo API cho màn Menu chính.
 *
 *  - `UpdateMenuScreen`: wrapper gọi từ vòng lặp main để xử lý input menu.
 *  - `ProcessMenuInput`: thực hiện logic di chuyển/ chọn mục.
 *  - `RenderMenuScreen`: vẽ danh sách menu và hiệu ứng tiêu đề.
 */

/** @brief Wrapper cập nhật trạng thái Menu từ vòng lặp thông điệp.
 *  @param currentState Tham chiếu trạng thái màn (có thể đổi màn hình khi chọn).
 *  @param selectedOption Tham chiếu chỉ mục mục đang chọn trong UI.
 *  @param wParam Mã phím/flags (WM_KEY/WM_CHAR encoded).
 */
void UpdateMenuScreen(ScreenState &currentState, int &selectedOption, WPARAM wParam);

/** @brief Xử lý logic phím cho menu và cập nhật lựa chọn.
 *  @param wParam Mã phím/flags nhận từ vòng lặp chính.
 *  @param currentState Tham chiếu trạng thái màn hình.
 *  @param selectedOption Tham chiếu tới chỉ mục mục đang chọn.
 *  @return `true` nếu trạng thái/lựa chọn thay đổi.
 */
bool ProcessMenuInput(WPARAM wParam, ScreenState &currentState, int &selectedOption);

/** @brief Vẽ giao diện màn Menu.
 *  @param hdc Device context để vẽ.
 *  @param selectedOption Mục đang chọn (sử dụng để highlight).
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 */
void RenderMenuScreen(HDC hdc, int selectedOption, int screenWidth, int screenHeight);

#endif // MENU_SCREEN_H