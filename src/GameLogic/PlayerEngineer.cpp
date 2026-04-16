#include "PlayerEngineer.h"

// ============================================================
//  initPlayer
//  Khởi tạo toàn bộ thông tin người chơi.
//  movesCount và totalWins đều về 0 — GameEngine chịu trách nhiệm
//  tăng/giảm trong suốt ván đấu.
// ============================================================
void initPlayer(PlayerInfo2& player, const wstring& name,
    const string& avatar, char piece, float maxTime)
{
    player.name = name;
    player.avatarPath = avatar;
    player.piece = piece;
    player.totalWins = 0;
    player.maxTurnTime = maxTime;
    player.movesCount = 0;
}

// ============================================================
//  resetPlayerForRound
//  Chỉ reset movesCount cho round mới trong Bo3/Bo5.
//  Không đụng totalWins — do GameEngine kiểm soát.
//
//  NOTE: recordWin() và updateMoveStats() đã bị xóa vì trùng
//  với logic trong GameEngine::processMove() và undoMove().
//  Nguồn sự thật duy nhất là GameEngine.
// ============================================================
void resetPlayerForRound(PlayerInfo2& player) {
    player.movesCount = 0;
}