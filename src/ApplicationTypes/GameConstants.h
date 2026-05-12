#ifndef _GAMECONSTANTS_H_
#define _GAMECONSTANTS_H_

/** @file GameConstants.h
 *  @brief Hằng số và macro cấu hình dùng toàn cục trong game.
 */

/** @brief Kích thước bàn mặc định. */
#define GOMOKU_SIZE 15

/** @brief Số quân liên tiếp để thắng (Caro = 5). */
#define WIN_CONDITION 5

/** @brief Kích thước tối đa mảng bàn để đảm bảo an toàn truy cập. */
#define MAX_BOARD_SIZE 20

/** @brief Giá trị ô trống. */
#define CELL_EMPTY 0

/** @brief Mã quân Player 1. */
#define CELL_PLAYER1 1

/** @brief Mã quân Player 2. */
#define CELL_PLAYER2 2

/** @brief Số slot lưu tối đa. */
#define MAX_SAVE_SLOTS 5

/** @brief Độ dài tối đa tên lưu (ký tự hiển thị). */
#define MAX_SAVE_NAME_LEN 15

/** @brief Tên hiển thị của ứng dụng. */
#define APP_TITLE L"CARO: Champions League"

/** @brief Kích thước cơ sở của title bar (px, trước khi scale). */
#define TITLE_BAR_BASE_HEIGHT 40

/** @brief Kích thước icon trong title bar (px, trước khi scale). */
#define TITLE_BAR_ICON_SIZE 24

/** @brief Kích thước nút điều khiển trong title bar (px, trước khi scale). */
#define TITLE_BAR_BUTTON_SIZE 24

/** @brief Padding ngang của title bar (px, trước khi scale). */
#define TITLE_BAR_PADDING_X 12

/** @brief Khoảng cách giữa các nút title bar (px, trước khi scale). */
#define TITLE_BAR_BUTTON_GAP 6

/** @brief Khoảng cách giữa icon và title text (px, trước khi scale). */
#define TITLE_BAR_TEXT_GAP 8

/** @brief Khoảng cách giữa title text và FPS text (px, trước khi scale). */
#define TITLE_BAR_FPS_GAP 12

/** @brief Độ dày nét vẽ icon nút title bar (px, trước khi scale). */
#define TITLE_BAR_GLYPH_THICKNESS 2

/** @brief Padding cho nét icon nút title bar (px, trước khi scale). */
#define TITLE_BAR_GLYPH_PADDING 6

// ===== Cửa sổ =====

/** @brief Kích thước cửa sổ mặc định khi khởi tạo (px). */
#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

// ===== Font =====

/** @brief Kích thước font cơ sở (px, trước UIScaler::S()). */
#define FONT_SIZE_DEFAULT 36
#define FONT_SIZE_BOLD 42
#define FONT_SIZE_TITLE 64
#define FONT_SIZE_NOTE 28

/** @brief Tên font family sử dụng trong toàn project. */
#define FONT_FAMILY_NAME L"VT323"

// ===== Input Throttling =====

/** @brief Thời gian tối thiểu giữa 2 lần nhấn phím (ms). */
#define KEY_THROTTLE_PRESS_MS 80

/** @brief Thời gian tối thiểu giữa 2 lần giữ phím (ms). */
#define KEY_THROTTLE_REPEAT_MS 150

/** @brief Thời gian tối thiểu giữa 2 lần phát SFX cùng alias (ms). */
#define SFX_COOLDOWN_MS 50

// ===== Gameplay =====

/** @brief Kích thước bàn Tic-Tac-Toe. */
#define TTT_BOARD_SIZE 3

/** @brief Thời gian đếm ngược tối thiểu và tối đa (giây). */
#define COUNTDOWN_MIN_SECONDS 10
#define COUNTDOWN_MAX_SECONDS 60
#define COUNTDOWN_STEP_SECONDS 5

/** @brief Điểm số mục tiêu (Bo) tối thiểu và tối đa. */
#define TARGET_SCORE_MIN 1
#define TARGET_SCORE_MAX 5
#define TARGET_SCORE_STEP 2

/** @brief Độ khó bot: min/max. */
#define BOT_DIFFICULTY_MIN 1
#define BOT_DIFFICULTY_MAX 3

/** @brief Số avatar có sẵn. */
#define TOTAL_AVATAR_COUNT 3

// ===== Audio =====

/** @brief Kích thước hàng đợi SFX tối đa. */
#define SFX_QUEUE_MAX_SIZE 32

/** @brief Thời gian hiển thị thông báo lưu game (giây). */
#define SAVE_FEEDBACK_DURATION 1.5f

#endif
