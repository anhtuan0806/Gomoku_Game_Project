#ifndef _PLAYER_ENGINEER_H_
#define _PLAYER_ENGINEER_H_

#include "../ApplicationTypes/PlayState.h"

// Khởi tạo thông tin người chơi (gọi 1 lần khi setup match)
void initPlayer(PlayerInfo2& player, const wstring& name,
    const string& avatar, char piece, float maxTime);

// Reset thống kê lượt (movesCount) cho round mới.
// totalWins KHÔNG reset ở đây — do initNewMatch quản lý.
void resetPlayerForRound(PlayerInfo2& player);

#endif // _PLAYER_ENGINEER_H_