#ifndef _PLAYER_ENGINEER_H_
#define _PLAYER_ENGINEER_H_

#include "../ApplicationTypes/PlayState.h"

// Khởi tạo thông tin người chơi (gọi 1 lần khi setup match)
void initializePlayer(PlayerMatchInfo &player, const wstring &name,
                      const string &avatar, char piece, float maxTime);

// Reset thống kê lượt (movesCount) cho round mới.
// totalWins KHÔNG reset ở đây — do initializeNewMatch quản lý.
void resetPlayerForRound(PlayerMatchInfo &player);

#endif // _PLAYER_ENGINEER_H_