#include "PlayerEngineer.h"

void initPlayer(PlayerInfo2& player, const wstring& name, const string& avatar, char piece, float maxTime)
{
    player.name = name;
    player.avatarPath = avatar;
    player.piece = piece;
    player.totalWins = 0;
    player.maxTurnTime = maxTime;
    player.movesCount = 0;
}

void resetPlayerForNewMatch(PlayerInfo2& player)
{
    player.movesCount = 0;
}

void recordWin(PlayerInfo2& player)
{
    player.totalWins++;
}

void updateMoveStats(PlayerInfo2& player)
{
    player.movesCount++;
}
