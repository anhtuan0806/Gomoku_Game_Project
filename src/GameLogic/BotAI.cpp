#include "BotAI.h"
#include "GameRules.h"
#include <cstdlib>
#include <climits>
#include <vector>
#include <algorithm>

using namespace std;

// Cấu trúc nội bộ để đánh giá các đường quân cờ
struct LineInfo {
    int count;
    int openEnds;
};

struct MoveCandidate {
    int r, c, score;
};

static bool isInBounds(int r, int c, int size) {
    return r >= 0 && r < size && c >= 0 && c < size;
}

static LineInfo analyzeDirection(const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int row, int col, int dr, int dc, int player) {
    LineInfo res = { 1, 0 };
    int r = row + dr, c = col + dc;
    while (isInBounds(r, c, size) && board[r][c] == player) {
        res.count++;
        r += dr; c += dc;
    }
    if (isInBounds(r, c, size) && board[r][c] == CELL_EMPTY) res.openEnds++;

    r = row - dr; c = col - dc;
    while (isInBounds(r, c, size) && board[r][c] == player) {
        res.count++;
        r -= dr; c -= dc;
    }
    if (isInBounds(r, c, size) && board[r][c] == CELL_EMPTY) res.openEnds++;

    return res;
}

static int evaluateLine(const LineInfo& line, int winLen) {
    if (line.count >= winLen) { 
        return 1000000; 
    }
    if (line.openEnds == 0) { 
        return 0; 
    }
    bool openTwo = (line.openEnds == 2);
    switch (line.count) {
    case 4: 
        return openTwo ? 50000 : 10000;
    case 3: 
        return openTwo ? 5000 : 1000;
    case 2: 
        return openTwo ? 500 : 100;
    case 1: 
        return openTwo ? 50 : 10;
    default: 
        return 0;
    }
}

static int scoreMove(const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int winLen, int row, int col, int player) {
    int total = 0;
    const int dirs[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
    for (int d = 0; d < 4; d++) {
        LineInfo line = analyzeDirection(board, size, row, col, dirs[d][0], dirs[d][1], player);
        total += evaluateLine(line, winLen);
    }
    return total;
}

static bool isCandidate(const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int r, int c) {
    int radius = 2;
    for (int dr = -radius; dr <= radius; dr++) {
        for (int dc = -radius; dc <= radius; dc++) {
            int nr = r + dr, nc = c + dc;
            if (isInBounds(nr, nc, size) && board[nr][nc] != CELL_EMPTY) { 
                return true; 
            }
        }
    }
    return false;
}

static void randomMove(const PlayState* state, int& outRow, int& outCol) {
    int size = state->boardSize;
    for (int attempts = 0; attempts < size * size; attempts++) {
        int r = rand() % size;
        int c = rand() % size;
        if (state->board[r][c] == CELL_EMPTY) {
            outRow = r; outCol = c; return;
        }
    }
    for (int r = 0; r < size; r++)
        for (int c = 0; c < size; c++)
            if (state->board[r][c] == CELL_EMPTY) {
                outRow = r; outCol = c; return;
            }
}

static void simpleMove(const PlayState* state, int& outRow, int& outCol) {
    int bestScore = -1;
    int size = state->boardSize;
    int winLen = (state->gameMode == MODE_CARO) ? 5 : 3;

    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            if (state->board[r][c] != CELL_EMPTY || !isCandidate(state->board, size, r, c)) { 
                continue; 
            }
            int atk = scoreMove(state->board, size, winLen, r, c, CELL_PLAYER2);
            int def = scoreMove(state->board, size, winLen, r, c, CELL_PLAYER1);
            int total = atk + def * 2;
            if (total > bestScore) {
                bestScore = total;
                outRow = r; outCol = c;
            }
        }
    }
    if (bestScore == -1) {
        randomMove(state, outRow, outCol);
    }
}

static int evaluateBoard(const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int winLen) {
    int score = 0;
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            if (board[r][c] == CELL_PLAYER2) {
                score += scoreMove(board, size, winLen, r, c, CELL_PLAYER2);
            } 
            else if (board[r][c] == CELL_PLAYER1) {
                score -= scoreMove(board, size, winLen, r, c, CELL_PLAYER1);
            }
        }
    }
    return score;
}

// Alpha-Beta with Move Ordering
static int alphaBeta(int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int winLen, int depth, int alpha, int beta, bool isMax) {
    if (depth == 0) return evaluateBoard(board, size, winLen);

    vector<MoveCandidate> moves;
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            if (board[r][c] == CELL_EMPTY && isCandidate(board, size, r, c)) {
                int atk = scoreMove(board, size, winLen, r, c, isMax ? CELL_PLAYER2 : CELL_PLAYER1);
                int def = scoreMove(board, size, winLen, r, c, isMax ? CELL_PLAYER1 : CELL_PLAYER2);
                moves.push_back({ r, c, atk + def });
            }
        }
    }

    sort(moves.begin(), moves.end(), [](const MoveCandidate& a, const MoveCandidate& b) {
        return a.score > b.score;
    });

    if (moves.empty()) {
        return evaluateBoard(board, size, winLen);
    }

    if (isMax) {
        int best = INT_MIN;
        for (const auto& m : moves) {
            board[m.r][m.c] = CELL_PLAYER2;
            int val = alphaBeta(board, size, winLen, depth - 1, alpha, beta, false);
            board[m.r][m.c] = CELL_EMPTY;
            if (val > best) {
                best = val;
            }
            if (best > alpha) {
                alpha = best;
            }
            if (beta <= alpha) {
                break;
            }
        }
        return best;
    } 
    else {
        int best = INT_MAX;
        for (const auto& m : moves) {
            board[m.r][m.c] = CELL_PLAYER1;
            int val = alphaBeta(board, size, winLen, depth - 1, alpha, beta, true);
            board[m.r][m.c] = CELL_EMPTY;
            if (val < best) {
                best = val;
            }
            if (best < beta) {
                beta = best;
            }
            if (beta <= alpha) {
                break;
            }
        }
        return best;
    }
}

void calculateAIMove(const PlayState* state, int difficulty, int& outRow, int& outCol) {
    if (difficulty == 1) {
        randomMove(state, outRow, outCol);
    } 
    else if (difficulty == 2) {
        simpleMove(state, outRow, outCol);
    } 
    else {
        int size = state->boardSize;
        int winLen = (state->gameMode == MODE_CARO) ? 5 : 3;
        int depth = 4; // Tăng depth nhờ Move Ordering

        int virtualBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
        for (int r = 0; r < MAX_BOARD_SIZE; r++) {
            for (int c = 0; c < MAX_BOARD_SIZE; c++) {
                virtualBoard[r][c] = state->board[r][c] != CELL_EMPTY ? state->board[r][c] : CELL_EMPTY;
            }
        }

        vector<MoveCandidate> moves;
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                if (virtualBoard[r][c] == CELL_EMPTY && isCandidate(virtualBoard, size, r, c)) {
                    int atk = scoreMove(virtualBoard, size, winLen, r, c, CELL_PLAYER2);
                    int def = scoreMove(virtualBoard, size, winLen, r, c, CELL_PLAYER1);
                    moves.push_back({ r, c, atk + def * 2 }); // Phòng thủ cao hơn 1 xíu ở nước đầu
                }
            }
        }

        sort(moves.begin(), moves.end(), [](const MoveCandidate& a, const MoveCandidate& b) {
            return a.score > b.score;
        });

        int bestScore = INT_MIN;
        if (!moves.empty()) {
            for (const auto& m : moves) {
                virtualBoard[m.r][m.c] = CELL_PLAYER2;
                int score = alphaBeta(virtualBoard, size, winLen, depth - 1, INT_MIN, INT_MAX, false);
                virtualBoard[m.r][m.c] = CELL_EMPTY;
                
                if (score > bestScore) {
                    bestScore = score;
                    outRow = m.r;
                    outCol = m.c;
                }
            }
        }

        if (bestScore == INT_MIN) {
            simpleMove(state, outRow, outCol);
        }
    }
}