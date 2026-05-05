#ifndef _GAME_RULES_H_
#define _GAME_RULES_H_

#include <vector>
#include <utility>
#include "../ApplicationTypes/PlayState.h"

/** @file GameRules.h
 *  @brief Hàm kiểm tra luật chơi: hợp lệ nước đi và điều kiện thắng.
 */

/**
 * @brief Kiểm tra ô (row,col) có hợp lệ để đặt quân hay không.
 * @return `true` nếu ô trống và nằm trong biên bàn.
 */
bool isValidMove(const PlayState *state, int row, int col);

/**
 * @brief Kiểm tra trạng thái trận đấu sau nước đi tại (lastRow, lastCol).
 *
 * @param state Trạng thái ván chơi hiện tại.
 * @param lastRow Hàng của nước đi vừa đặt.
 * @param lastCol Cột của nước đi vừa đặt.
 * @param outWinCells Nếu != nullptr, được điền danh sách ô tạo thành chuỗi thắng.
 * @return `CELL_PLAYER1` hoặc `CELL_PLAYER2` khi có người thắng,
 *         `CELL_EMPTY` nếu hòa, hoặc `-1` nếu trận chưa kết thúc.
 */
int checkWinCondition(const PlayState *state, int lastRow, int lastCol,
                      std::vector<std::pair<int, int>> *outWinCells = nullptr);

#endif // _GAME_RULES_H_