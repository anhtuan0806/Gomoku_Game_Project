#ifndef _PLAYER_ENGINEER_H_
#define _PLAYER_ENGINEER_H_

#include "../ApplicationTypes/PlayState.h"

// Khởi tạo thông tin người chơi
void initPlayer(PlayerInfo2& player, const wstring& name, const string& avatar, char piece, float maxTime);

// Reset thông tin người chơi cho ván đấu mới
void resetPlayerForNewMatch(PlayerInfo2& player);

// Ghi nhận chiến thắng
void recordWin(PlayerInfo2& player);

// Cập nhật số nước đi
void updateMoveStats(PlayerInfo2& player);


#endif