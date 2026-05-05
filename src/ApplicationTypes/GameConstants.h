#ifndef _GAMECONSTANTS_H_
#define _GAMECONSTANTS_H_

/** @file GameConstants.h
 *  @brief Hằng số và macro cấu hình dùng toàn cục trong game.
 */

/** @brief Kích thước bàn mặc định. */
#define GOMOKU_SIZE 15

/** @brief Số quân liên tiếp để thắng (Caro = 5). */
#define WIN_CONDITION 5

/** @brief Kích thước tối đa mảng bàn để đảm bảo an toàn truy cập. */
#define MAX_BOARD_SIZE 20

/** @brief Giá trị ô trống. */
#define CELL_EMPTY 0

/** @brief Mã quân Player 1. */
#define CELL_PLAYER1 1

/** @brief Mã quân Player 2. */
#define CELL_PLAYER2 2

/** @brief Số slot lưu tối đa. */
#define MAX_SAVE_SLOTS 5

/** @brief Độ dài tối đa tên lưu (ký tự hiển thị). */
#define MAX_SAVE_NAME_LEN 15

#endif