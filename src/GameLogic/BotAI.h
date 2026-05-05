#ifndef _BOT_AI_H_
#define _BOT_AI_H_

#include "../ApplicationTypes/PlayState.h"
#include <unordered_map>
#include <cstdint>

/** @file BotAI.h
 *  @brief Công cụ AI cho Bot: Zobrist hashing, transposition table và search.
 */

/**
 * @brief Kích thước của bảng transposition table (số entry là 2^20).
 * @note Kích thước này cho khoảng ~1M entry (khoảng 32MB bộ nhớ cho TT).
 */
static constexpr int sTranspositionTableSize = 1 << 20;

/**
 * @brief Kiểu cờ cho entry trong Transposition Table.
 * @details Dùng để biểu diễn bản chất của giá trị lưu: chính xác, lower-bound hoặc upper-bound.
 */
enum TTFlag : std::uint8_t
{
    TT_EXACT = 0, /**< Giá trị chính xác (exact score) */
    TT_LOWER = 1, /**< Lower bound (alpha cutoff) */
    TT_UPPER = 2  /**< Upper bound (beta cutoff) */
};

/**
 * @brief Entry đơn cho Transposition Table.
 * @param hash Zobrist hash của trạng thái bàn cờ.
 * @param score Giá trị đánh giá được lưu.
 * @param depth Độ sâu search tương ứng với giá trị này.
 * @param flag Loại giá trị (exact/lower/upper).
 */
struct TTEntry
{
    std::uint64_t hash = 0;
    int score = 0;
    std::int8_t depth = 0;
    TTFlag flag = TT_EXACT;
};

/**
 * @name Public API
 * @{
 */

/**
 * @brief Tính toán nước đi cho Bot (Bot luôn là `PLAYER2`).
 *
 * @param state Trạng thái ván đang chơi (không bị hàm thay đổi).
 * @param difficulty Độ khó: 1 = Random, 2 = Simple heuristic, 3 = Alpha-Beta + TT.
 * @param outRow Tham chiếu trả về hàng của nước đi chọn.
 * @param outCol Tham chiếu trả về cột của nước đi chọn.
 */
void calculateComputerMove(const PlayState *state, int difficulty, int &outRow, int &outCol);

/**
 * @brief Xóa toàn bộ Transposition Table (dùng khi bắt đầu ván mới).
 */
void clearTranspositionTable();

/** @} */

#endif // _BOT_AI_H_