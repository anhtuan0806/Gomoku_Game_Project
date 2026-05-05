#ifndef _PLAYER_ENGINEER_H_
#define _PLAYER_ENGINEER_H_

#include "../ApplicationTypes/PlayState.h"

/** @file PlayerEngineer.h
 *  @brief Hàm khởi tạo và reset thông tin PlayerMatchInfo.
 */

/**
 * @brief Khởi tạo thông tin người chơi dùng trong `PlayState` (gọi khi setup match).
 * @param player Tham chiếu tới cấu trúc `PlayerMatchInfo` sẽ khởi tạo.
 * @param name Tên hiển thị (Unicode).
 * @param avatar Đường dẫn/khóa avatar.
 * @param piece Ký tự đại diện ('X' / 'O').
 * @param maxTime Thời gian tối đa cho mỗi nước (giây).
 */
void initializePlayer(PlayerMatchInfo &player, const std::wstring &name,
                      const std::string &avatar, char piece, float maxTime);

/**
 * @brief Reset các thống kê round-level (ví dụ `movesCount`) khi bắt đầu round mới.
 * @note `totalWins` KHÔNG bị reset bởi hàm này; `initializeNewMatch` chịu trách nhiệm đó.
 */
void resetPlayerForRound(PlayerMatchInfo &player);

#endif // _PLAYER_ENGINEER_H_