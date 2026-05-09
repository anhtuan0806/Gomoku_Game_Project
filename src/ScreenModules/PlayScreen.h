#ifndef _PLAY_SCREEN_H_
#define _PLAY_SCREEN_H_
#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../ApplicationTypes/GameConfig.h"
#include "../RenderAPI/Renderer.h"

/** @file PlayScreen.h
 *  @brief Khai báo các API cho màn chơi chính (Play Screen).
 *
 *  Chứa các hàm để cập nhật logic trận (`UpdatePlayLogic`), xử lý input (`ProcessPlayInput`),
 *  vẽ giao diện (`RenderPlayScreen`) và các hàm tiện ích liên quan.
 */

enum PauseSubMenu
{
	SUB_MAIN,
	SUB_SAVE_SELECT,
	SUB_SAVE_NAME_ENTRY
};

static int g_PauseSelected = 0;
const int TOTAL_PAUSE_ITEMS = 5;
static PauseSubMenu g_CurrentSubMenu = SUB_MAIN;
static int g_SaveSlotSelected = 0;
static std::wstring g_SaveNameInput = L"";

/** @brief Cập nhật logic trận theo delta-time.
 *  @param state Trạng thái trận đấu (`PlayState`) sẽ được cập nhật.
 *  @param dt Khoảng thời gian (giây) kể từ lần cập nhật trước.
 *  @return `true` nếu cần vẽ lại màn hình.
 */
bool UpdatePlayLogic(PlayState *state, double dt);

/** @brief Xử lý sự kiện phím cho màn chơi.
 *  @param wParam Mã phím/flags (WM_KEY/WM_CHAR encoded).
 *  @param state Trạng thái trận đấu để đọc/ghi.
 *  @param currentState Tham chiếu trạng thái màn (có thể đổi sang `SCREEN_MENU`, `SCREEN_PLAY`, ...).
 *  @param config Cấu hình game (BGM/SFX bật/tắt, âm lượng...).
 *  @return `true` nếu trạng thái hoặc UI thay đổi.
 */
bool ProcessPlayInput(WPARAM wParam, PlayState *state, ScreenState &currentState, GameConfig *config);

/** @brief Vẽ màn Play lên `hdc`.
 *  @param hdc Device context để vẽ.
 *  @param state Trạng thái trận đấu để hiển thị.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 *  @param config Cấu hình game để hiển thị các tuỳ chọn (ví dụ BGM state).
 */
void RenderPlayScreen(HDC hdc, const PlayState *state, int screenWidth, int screenHeight, const GameConfig *config, const RECT *clip = nullptr);

/** @brief Wrapper input từ main loop tới Play screen.
 *  @param state Trạng thái trận đấu.
 *  @param currentState Tham chiếu trạng thái màn để chuyển màn nếu cần.
 *  @param wParam Mã phím/flags nhận từ main loop.
 *  @param config Cấu hình game.
 */
void UpdatePlayScreen(PlayState *state, ScreenState &currentState, WPARAM wParam, GameConfig *config);

/** @brief Reset các biến static của Play screen về trạng thái mặc định.
 *  Gọi khi bắt đầu hoặc kết thúc 1 phiên chơi để đảm bảo trạng thái tĩnh được tái khởi tạo.
 */
void ResetPlayScreenStatics();

#endif // _PLAY_SCREEN_H_