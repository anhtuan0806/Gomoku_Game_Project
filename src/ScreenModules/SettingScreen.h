#ifndef SETTING_SCREEN_H
#define SETTING_SCREEN_H

#include <windows.h>
#include "../SystemModules/ConfigLoader.h"
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/GameConfig.h"

/** @file SettingScreen.h
 *  @brief API cho màn cài đặt (Settings).
 *
 *  Bao gồm các hàm xử lý input, cập nhật trạng thái và vẽ UI cho cài đặt.
 */

/** @brief Xử lý thay đổi cài đặt (nội bộ).
 *  @param currentState Tham chiếu trạng thái màn để có thể chuyển màn.
 *  @param config Con trỏ cấu hình game để chỉnh sửa các tuỳ chọn.
 *  @param selectedOption Chỉ mục tuỳ chọn đang chọn.
 *  @param direction Hướng di chuyển trong menu (ví dụ -1/1).
 *  @param isEnterPressed true nếu nút xác nhận được nhấn.
 *  @param isRepeat true nếu phím đang được giữ để tự lặp.
 */
void ProcessSettingInput(ScreenState &currentState, GameConfig *config, int selectedOption, int direction, bool isEnterPressed, bool isRepeat);

/** @brief Cập nhật trạng thái màn Settings.
 *  @param currentState Tham chiếu trạng thái màn.
 *  @param config Con trỏ tới `GameConfig` để áp dụng thay đổi.
 *  @param selectedOption Tham chiếu chỉ mục đang chọn (được sửa trong hàm).
 *  @param keyCode Mã phím nhận từ vòng lặp chính.
 */
void UpdateSettingScreen(ScreenState &currentState, GameConfig *config, int &selectedOption, WPARAM keyCode);

/** @brief Vẽ giao diện màn Settings.
 *  @param hdc Device context để vẽ.
 *  @param config Trạng thái cấu hình hiện tại.
 *  @param selectedOption Mục đang chọn để highlight.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 */
void RenderSettingScreen(HDC hdc, const GameConfig *config, int selectedOption, int screenWidth, int screenHeight);

#endif // SETTING_SCREEN_H