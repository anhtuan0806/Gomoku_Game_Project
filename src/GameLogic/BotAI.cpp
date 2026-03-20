#include "BotAI.h"
#include "GameRules.h"
#include <cstdlib>
#include <climits>

// --- CÁC HÀM TRỢ GIÚP TĨNH ---

static bool isInBounds(int r, int c, int size) {
    return r >= 0 && r < size && c >= 0 && c < size;
}

// Cấu trúc nội bộ để đánh giá các đường quân cờ
struct LineInfo {
    int count;    // Số quân liên tiếp
    int openEnds; // Số đầu trống (0, 1, hoặc 2)
};

// Phân tích một hướng cụ thể từ một ô cờ
static LineInfo analyzeDirection(const PlayState* state, int row, int col, int dr, int dc, int player) {
    LineInfo res = { 1, 0 };
    int size = state->boardSize;

    // Kiểm tra hướng tiến
    int r = row + dr, c = col + dc;
    while (isInBounds(r, c, size) && state->board[r][c] == player) {
        res.count++;
        r += dr; c += dc;
    }
    if (isInBounds(r, c, size) && state->board[r][c] == CELL_EMPTY)
        res.openEnds++;

    // Kiểm tra hướng lùi
    r = row - dr; c = col - dc;
    while (isInBounds(r, c, size) && state->board[r][c] == player) {
        res.count++;
        r -= dr; c -= dc;
    }
    if (isInBounds(r, c, size) && state->board[r][c] == CELL_EMPTY)
        res.openEnds++;

    return res;
}

// Tính điểm cho một đường dựa trên luật Gomoku
static int evaluateLine(const LineInfo& line, int winLen) {
    if (line.count >= winLen) return 1000000; // Thắng tuyệt đối
    if (line.openEnds == 0) return 0;         // Bị chặn 2 đầu thì vô dụng

    bool openTwo = (line.openEnds == 2);
    switch (line.count) {
    case 4: return openTwo ? 50000 : 10000;
    case 3: return openTwo ? 5000 : 1000;
    case 2: return openTwo ? 500 : 100;
    case 1: return openTwo ? 50 : 10;
    default: return 0;
    }
}

// Tính tổng điểm cho một ô cờ nếu đánh vào đó
static int scoreMove(const PlayState* state, int row, int col, int player) {
    int total = 0;
    int winLen = (state->gameMode == MODE_CARO) ? 5 : 3;
    const int dirs[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };

    for (int d = 0; d < 4; d++) {
        LineInfo line = analyzeDirection(state, row, col, dirs[d][0], dirs[d][1], player);
        total += evaluateLine(line, winLen);
    }
    return total;
}

// Chỉ xét các ô cờ gần các ô đã đánh để tăng tốc độ xử lý (Candidate check)
static bool isCandidate(const PlayState* state, int r, int c) {
    int radius = 2;
    for (int dr = -radius; dr <= radius; dr++) {
        for (int dc = -radius; dc <= radius; dc++) {
            int nr = r + dr, nc = c + dc;
            if (isInBounds(nr, nc, state->boardSize) && state->board[nr][nc] != CELL_EMPTY)
                return true;
        }
    }
    return false;
}

// --- CÁC CHIẾN THUẬT AI ---

// 1. AI Dễ: Ngẫu nhiên
static void randomMove(const PlayState* state, int& outRow, int& outCol) {
    int size = state->boardSize;
    for (int attempts = 0; attempts < size * size; attempts++) {
        int r = rand() % size;
        int c = rand() % size;
        if (state->board[r][c] == CELL_EMPTY) {
            outRow = r; outCol = c; return;
        }
    }
    // Fallback: Duyệt tuần tự
    for (int r = 0; r < size; r++)
        for (int c = 0; c < size; c++)
            if (state->board[r][c] == CELL_EMPTY) { outRow = r; outCol = c; return; }
}

// 2. AI Trung bình: Tham lam 
static void simpleMove(const PlayState* state, int& outRow, int& outCol) {
    int bestScore = -1;
    int size = state->boardSize;

    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            if (state->board[r][c] != CELL_EMPTY || !isCandidate(state, r, c)) continue;

            int atk = scoreMove(state, r, c, CELL_PLAYER2); // Bot tấn công
            int def = scoreMove(state, r, c, CELL_PLAYER1); // Bot phòng thủ

            int total = atk + def * 2; // Ưu tiên chặn người chơi hơn một chút
            if (total > bestScore) {
                bestScore = total;
                outRow = r; outCol = c;
            }
        }
    }
    if (bestScore == -1) randomMove(state, outRow, outCol);
}

// 3. AI Khó: Alpha-Beta Pruning
static int evaluateBoard(PlayState* state) {
    int score = 0;
    for (int r = 0; r < state->boardSize; r++) {
        for (int c = 0; c < state->boardSize; c++) {
            if (state->board[r][c] == CELL_PLAYER2) score += scoreMove(state, r, c, CELL_PLAYER2);
            else if (state->board[r][c] == CELL_PLAYER1) score -= scoreMove(state, r, c, CELL_PLAYER1);
        }
    }
    return score;
}

static int alphaBeta(PlayState* state, int depth, int alpha, int beta, bool isMax) {
    if (depth == 0) return evaluateBoard(state);

    int size = state->boardSize;
    if (isMax) {
        int best = INT_MIN;
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                if (state->board[r][c] == CELL_EMPTY && isCandidate(state, r, c)) {
                    state->board[r][c] = CELL_PLAYER2;
                    int val = alphaBeta(state, depth - 1, alpha, beta, false);
                    state->board[r][c] = CELL_EMPTY;
                    if (val > best) best = val;
                    if (best > alpha) alpha = best;
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    }
    else {
        int best = INT_MAX;
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                if (state->board[r][c] == CELL_EMPTY && isCandidate(state, r, c)) {
                    state->board[r][c] = CELL_PLAYER1;
                    int val = alphaBeta(state, depth - 1, alpha, beta, true);
                    state->board[r][c] = CELL_EMPTY;
                    if (val < best) best = val;
                    if (best < beta) beta = best;
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    }
}

// --- ENTRY POINT CHÍNH ---

void calculateAIMove(const PlayState* state, int difficulty, int& outRow, int& outCol) {
    if (difficulty == 1) {
        randomMove(state, outRow, outCol);
    }
    else if (difficulty == 2) {
        simpleMove(state, outRow, outCol);
    }
    else {
        // Tạm thời ép kiểu sang non-const để thuật toán Alpha-Beta thử nước đi (sau đó reset)
        PlayState* mutableState = const_cast<PlayState*>(state);
        int bestScore = INT_MIN;
        int depth = (state->boardSize <= 5) ? 4 : 2; // Giảm độ sâu bàn cờ lớn để tránh lag

        for (int r = 0; r < state->boardSize; r++) {
            for (int c = 0; c < state->boardSize; c++) {
                if (state->board[r][c] == CELL_EMPTY && isCandidate(state, r, c)) {
                    mutableState->board[r][c] = CELL_PLAYER2;
                    int score = alphaBeta(mutableState, depth, INT_MIN, INT_MAX, false);
                    mutableState->board[r][c] = CELL_EMPTY;
                    if (score > bestScore) {
                        bestScore = score;
                        outRow = r; outCol = c;
                    }
                }
            }
        }
        if (bestScore == INT_MIN) simpleMove(state, outRow, outCol);
    }
}