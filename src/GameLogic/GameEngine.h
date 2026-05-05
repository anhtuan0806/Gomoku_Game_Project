#ifndef _GAME_ENGINE_H_
#define _GAME_ENGINE_H_

#include "../ApplicationTypes/PlayState.h"

/** @file GameEngine.h
 *  @brief API quản lý vòng đời ván chơi: khởi tạo, xử lý nước đi, undo/redo, đổi lượt.
 */

/**
 * @brief Khởi tạo một trận đấu mới và thiết lập trạng thái `PlayState`.
 *
 * @param state Con trỏ tới `PlayState` sẽ được khởi tạo.
 * @param mode Kiểu chơi (`PlayMode`).
 * @param type Loại trận (`MatchType`).
 * @param boardSize Kích thước bảng (ví dụ 15, 19).
 * @param countdownTime Thời gian đếm ngược cho mỗi nước (giây).
 * @param difficulty Mức độ khó (dùng khi có Bot).
 * @param targetScore Số điểm mục tiêu (cho chế độ nhiều round).
 * @param totalTime Tổng thời gian trận (mặc định 0 = không giới hạn).
 */
void initializeNewMatch(PlayState *state, PlayMode mode, MatchType type, int boardSize,
                        int countdownTime, int difficulty, int targetScore, int totalTime = 0);

/**
 * @brief Reset bàn cờ cho round mới, giữ nguyên điểm số hiện có.
 * @param state Trạng thái ván chơi hiện tại.
 */
void startNextRound(PlayState *state);

/**
 * @brief Thực hiện nước đi tại (row,col), cập nhật trạng thái, kiểm tra thắng/thua.
 * @param state Trạng thái ván chơi.
 * @param row Hàng vẽ nước đi.
 * @param col Cột vẽ nước đi.
 * @return `true` nếu nước đi hợp lệ và đã được áp dụng; `false` nếu không hợp lệ.
 */
bool processMove(PlayState *state, int row, int col);

/**
 * @brief Chuyển lượt chơi sang người/đối phương và reset bộ đếm thời gian.
 * @param state Trạng thái ván chơi.
 */
void switchTurn(PlayState *state);

/**
 * @brief Hoàn tác (undo) một nước đi.
 * @param state Trạng thái ván chơi.
 */
void undoMove(PlayState *state);

/**
 * @brief Làm lại (redo) một nước đi đã undo.
 * @param state Trạng thái ván chơi.
 */
void redoMove(PlayState *state);

#endif // _GAME_ENGINE_H_