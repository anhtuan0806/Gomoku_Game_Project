#include "AI_PvE.h"
#include "MatchRules.h"
#include <cstdlib> // Cho hàm rand()

void CalculateAIMove(const PlayState* state, uint8_t& outX, uint8_t& outY) {
    uint8_t size = state->boardSize;

    // Thuật toán ĐƠN GIẢN: Tìm ô trống đầu tiên đánh vào (nâng cấp thuật toán Minimax sau)

    uint8_t attempts = 0;
    while (attempts < size * size) {
        uint8_t rX = rand() % size;
        uint8_t rY = rand() % size;

        if (state->board[rY][rX].c == 0) {
            outX = rX;
            outY = rY;
            return;
        }
        attempts++;
    }

    // Fallback: Duyệt tuần tự nếu random quá lâu
    for (uint8_t y = 0; y < size; y++) {
        for (uint8_t x = 0; x < size; x++) {
            if (state->board[y][x].c == 0) {
                outX = x;
                outY = y;
                return;
            }
        }
    }
}