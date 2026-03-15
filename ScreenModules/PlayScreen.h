#ifndef PLAY_SCREEN_H
#define PLAY_SCREEN_H

#include <windows.h>
#include <string>
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../RenderAPI/Renderer.h"

const int CELL_SIZE = 40;

/**
 * @brief Cập nhật logic tự động (Thời gian đếm ngược, AI tự động đánh).
 * @return bool Trả về true nếu trạng thái thay đổi cần vẽ lại màn hình.
 */
bool UpdatePlayLogic(PlayState* state, double dt);

/**
 * @brief Xử lý sự kiện bàn phím của người chơi (WM_KEYDOWN).
 * @return bool Trả về true nếu người dùng có thao tác yêu cầu vẽ lại.
 */
bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState);

/**
 * @brief Kết xuất bàn cờ, UI và hiệu ứng lên màn hình.
 */
void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO);

/**
 * @brief Hàm giao tiếp chính để cập nhật đầu vào (Input) của màn hình Play từ sự kiện WM_KEYDOWN.
 */
void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam);

void InitNewMatch(PlayState* state, PlayMode mode, MatchType type, int boardSize, int countdownTime);
#endif // PLAY_SCREEN_H