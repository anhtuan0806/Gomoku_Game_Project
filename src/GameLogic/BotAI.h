#ifndef _BOT_AI_H_
#define _BOT_AI_H_

#include "../ApplicationTypes/PlayState.h"
#include <unordered_map>
#include <cstdint>

// ============================================================
//  Transposition Table (Zobrist Hashing)
// ============================================================

// Kích thước bảng transposition (2^20 ~ 1M entry, ~32MB RAM)
static constexpr int TT_SIZE = 1 << 20;

enum TTFlag : uint8_t {
    TT_EXACT = 0,   // Giá trị chính xác
    TT_LOWER = 1,   // Alpha cutoff  (lower bound)
    TT_UPPER = 2    // Beta  cutoff  (upper bound)
};

struct TTEntry {
    uint64_t hash = 0;
    int      score = 0;
    int8_t   depth = 0;
    TTFlag   flag = TT_EXACT;
};

// ============================================================
//  Incremental board evaluation
// ============================================================
// Lưu điểm tích lũy của cả hai bên để không scan lại toàn bàn.
// Được update mỗi khi đặt / xóa quân trong alpha-beta.
struct IncrementalScore {
    int p1 = 0; // Tổng điểm CELL_PLAYER1
    int p2 = 0; // Tổng điểm CELL_PLAYER2
};

// ============================================================
//  Public API
// ============================================================

// Tính toán tọa độ tốt nhất cho máy (Bot luôn là PLAYER2)
// difficulty: 1 (Dễ - Random), 2 (Trung bình), 3 (Khó - AlphaBeta + TT)
void calculateAIMove(const PlayState* state, int difficulty, int& outRow, int& outCol);

// Xóa transposition table (gọi khi bắt đầu ván mới)
void clearTranspositionTable();

#endif // _BOT_AI_H_