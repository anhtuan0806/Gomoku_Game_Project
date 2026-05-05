#ifndef MATCH_CONFIG_SCREEN_H
#define MATCH_CONFIG_SCREEN_H

#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"

/** @file MatchConfigScreen.h
 *  @brief Khai báo API cho màn cấu hình trận đấu (Match Config).
 *
 *  Chứa các hàm để xử lý input, cập nhật trạng thái cấu hình và vẽ giao diện cấu hình.
 */

/** @brief Xử lý sự kiện phím và cập nhật trạng thái màn cấu hình trận.
 *  @param currentState Tham chiếu trạng thái màn (có thể chuyển sang `SCREEN_PLAY`).
 *  @param playState Trạng thái cấu hình trận (`PlayState`) để đọc/ghi các tuỳ chọn.
 *  @param selectedOption Tham chiếu tới chỉ mục mục đang được chọn trong UI.
 *  @param wParam Mã phím/flags (WM_KEY/WM_CHAR encoded) do vòng lặp chính truyền vào.
 */
void UpdateMatchConfigScreen(ScreenState &currentState, PlayState *playState, int &selectedOption, WPARAM wParam);

/** @brief Vẽ màn cấu hình trận lên `hdc`.
 *  @param hdc Device context để vẽ.
 *  @param selectedOption Mục đang được chọn (dùng để highlight).
 *  @param config Trạng thái cấu hình hiện tại (giá trị hiển thị).
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 */
void RenderMatchConfigScreen(HDC hdc, int selectedOption, const PlayState *config, int screenWidth, int screenHeight);

#endif