#include "PlayerEngineer.h"

// (g_PlayState is now defined in ApplicationTypes/PlayState.cpp)


/** @file PlayerEngineer.cpp
 *  @brief Thực thi các thao tác khởi tạo và reset cho `PlayerMatchInfo`.
 */

/**
 * @brief Khởi tạo toàn bộ thông tin người chơi.
 * @note `movesCount` và `totalWins` được đặt về 0; `GameEngine` quản lý tăng/giảm trong ván.
 */
void initializePlayer(PlayerMatchInfo &player, const std::wstring &name,
                      const std::string &avatar, char piece, float maxTime)
{
    player.name = name;
    player.avatarPath = avatar;
    player.piece = piece;
    player.totalWins = 0;
    player.matchWins = 0;
    player.maxTurnTime = maxTime;
    player.movesCount = 0;
    player.totalTimePossessed = 0.0f;
}

/**
 * @brief Reset các chỉ số `round-level` (ví dụ `movesCount`) cho round mới.
 * @note `totalWins` và `matchWins` được giữ nguyên để theo dõi series BO.
 */
void resetPlayerForRound(PlayerMatchInfo &player)
{
    player.movesCount = 0;
}