#ifndef _GAME_STATE_H
#define _GAME_STATE_H
#include <string>

/** @file GameState.h
 *  @brief Kiểu trạng thái màn hình, chế độ chơi và kiểu trận.
 */

/** @brief Trạng thái màn hình UI chính. */
enum ScreenState
{
    SCREEN_MENU,         /**< Màn hình menu chính */
    SCREEN_MATCH_CONFIG, /**< Cấu hình trận đấu */
    SCREEN_PLAY,         /**< Màn hình chơi */
    SCREEN_LOAD_GAME,    /**< Màn hình tải game */
    SCREEN_SETTING,      /**< Màn hình cài đặt */
    SCREEN_EXIT,         /**< Thoát ứng dụng */
    SCREEN_GUIDE,        /**< Hướng dẫn */
    SCREEN_ABOUT         /**< Thông tin ứng dụng */
};

/** @brief Chế độ chơi: Caro hoặc Tic-Tac-Toe. */
enum PlayMode
{
    MODE_CARO,       /**< Chơi Caro (mặc định) */
    MODE_TIC_TAC_TOE /**< Chơi Tic-Tac-Toe */
};

/** @brief Loại trận: PVP hoặc PVE (vs Bot). */
enum MatchType
{
    MATCH_PVP, /**< Player vs Player */
    MATCH_PVE  /**< Player vs Environment (Bot) */
};

// --- Globals (Owned by GameState/Application Flow) ---
extern ScreenState g_CurrentScreen;

// Lựa chọn hiện tại trong các menu
extern int g_ConfigSelected;
extern int g_MenuSelected;
extern int g_LoadSelected;
extern int g_SettingSelected;
extern std::wstring g_LoadStatus;
extern int g_GuildPage;

/** @brief Kiểm tra xem màn hình có cần animation hay không. */
bool ShouldAnimateScreen(ScreenState screen);

#endif // _GAME_STATE_H