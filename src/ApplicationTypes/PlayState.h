#ifndef _PLAY_STATE_H
#define _PLAY_STATE_H
#include "GameConstants.h"
#include "GameState.h"
#include <string>
#include <vector>
#include <utility>

/** @file PlayState.h
 *  @brief Kiểu dữ liệu ứng dụng: trạng thái ván chơi và thông tin người chơi.
 */

/** @brief Trạng thái (lifecycle) của trận đấu. */
enum MatchStatus
{
    MATCH_PLAYING,  /**< Đang chơi */
    MATCH_PAUSED,   /**< Tạm dừng */
    MATCH_FINISHED, /**< Kết thúc ván */
    MATCH_SUMMARY   /**< Màn tóm tắt kết quả */
};

/** @brief Thông tin người chơi (per-match/player record). */
struct PlayerMatchInfo
{
    std::wstring name;               /**< Tên hiển thị (Unicode) */
    std::string avatarPath;          /**< Đường dẫn hoặc khóa avatar */
    char piece = ' ';                /**< Ký hiệu quân ('X' hoặc 'O') */
    int totalWins = 0;               /**< Số bàn thắng trong series hiện tại */
    int matchWins = 0;               /**< Số trận (series) đã thắng */
    int movesCount = 0;              /**< Số nước đã đi trong round hiện tại */
    float maxTurnTime = 0.0f;        /**< Thời gian tối đa cho 1 lượt (giây) */
    float totalTimePossessed = 0.0f; /**< Tổng thời gian giữ bóng (giây) */
};

/** @brief Toàn bộ trạng thái ván chơi: board, players, timers, lịch sử. */
struct PlayState
{
    PlayMode gameMode = MODE_CARO;   /**< Chế độ chơi (Caro / TicTacToe) */
    MatchType matchType = MATCH_PVP; /**< Kiểu trận (PVP / PVE) */

    PlayerMatchInfo player1; /**< Thông tin player 1 */
    PlayerMatchInfo player2; /**< Thông tin player 2 */

    bool isPlayer1Turn = true; /**< True = lượt P1 */

    int countdownTime = 30; /**< Thời gian mặc định cho mỗi lượt (giây) */
    int timeRemaining = 30; /**< Thời gian còn lại cho lượt hiện tại (giây) */

    int boardSize = MAX_BOARD_SIZE;                  /**< Kích thước bàn cờ (n x n) */
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE] = {0}; /**< Ma trận ô (CELL_*) */

    int cursorRow = 0; /**< Hàng con trỏ hiện tại */
    int cursorCol = 0; /**< Cột con trỏ hiện tại */

    int lastMoveRow = -1;                          /**< Hàng nước đi cuối cùng */
    int lastMoveCol = -1;                          /**< Cột nước đi cuối cùng */
    std::vector<std::pair<int, int>> winningCells; /**< Danh sách ô tạo thành chuỗi thắng */

    std::vector<std::pair<int, int>> matchHistory; /**< Lịch sử nước đi (stack) */
    std::vector<std::pair<int, int>> redoStack;    /**< Stack redo */

    MatchStatus status = MATCH_PLAYING; /**< Trạng thái trận */
    int winner = -1;                    /**< 0:Hòa, 1:P1, 2:P2, -1:Đang chơi */

    int difficulty = 2;  /**< 1, 2, 3 mức độ khó cho Bot */
    int targetScore = 1; /**< Số điểm mục tiêu cho series (1=BO1,2=BO3,3=BO5) */

    float matchDuration = 0.0f;   /**< Tổng thời gian trận (giây) */
    int player1TotalTimeLeft = 0; /**< Tổng thời gian còn lại P1 (giây) */
    int player2TotalTimeLeft = 0; /**< Tổng thời gian còn lại P2 (giây) */
    bool isMatchTimed = false;    /**< Có giới hạn tổng trận hay không */

    std::wstring saveName;      /**< Tên hiển thị của bản lưu */
    std::wstring saveTimestamp; /**< Thời gian lưu (Ngày/Giờ) */
};

// --- Globals (Owned by PlayState) ---
extern PlayState g_PlayState;

#endif // _PLAY_STATE_H