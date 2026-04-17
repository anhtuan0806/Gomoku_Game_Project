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
    player.matchWins = 0;
    player.maxTurnTime = maxTime;
    player.movesCount = 0;
    player.totalTimePossessed = 0.0f;
}

// ============================================================
//  resetPlayerForRound
//  Chỉ reset các chỉ số mang tính "thời điểm" cho round mới.
// ============================================================
void resetPlayerForRound(PlayerInfo2& player) {
    player.movesCount = 0;
    // Lưu ý: totalWins và matchWins được giữ nguyên để tính tiến trình series BO
}