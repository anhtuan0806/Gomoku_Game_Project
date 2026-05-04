#include "BotAI.h"
#include "GameRules.h"
#include <cstdlib>
#include <climits>
#include <cstring>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

// ============================================================
//  Zobrist Hashing
//  sZobristTable[row][col][piece]  piece: 0=P1, 1=P2
// ============================================================
static uint64_t sZobristTable[MAX_BOARD_SIZE][MAX_BOARD_SIZE][2];
static bool sIsZobristInitialized = false;

static void initializeZobrist()
{
    if (sIsZobristInitialized)
        return;
    mt19937_64 rng(0xDEADBEEFCAFEBABEULL);
        for (int row = 0; row < MAX_BOARD_SIZE; row++)
            for (int col = 0; col < MAX_BOARD_SIZE; col++)
                for (int pieceIdx = 0; pieceIdx < 2; pieceIdx++)
                    sZobristTable[row][col][pieceIdx] = rng();
    sIsZobristInitialized = true;
}

static inline uint64_t zobristHashForCell(int row, int col, int piece)
{
    // piece: CELL_PLAYER1=1 → idx 0, CELL_PLAYER2=2 → idx 1
    return sZobristTable[row][col][piece - 1];
}

// ============================================================
//  Transposition Table
// ============================================================
static TTEntry sTranspositionTable[sTranspositionTableSize];

void clearTranspositionTable()
{
    memset(sTranspositionTable, 0, sizeof(sTranspositionTable));
}

static inline TTEntry *lookupTranspositionEntry(uint64_t hash)
{
    return &sTranspositionTable[hash & (sTranspositionTableSize - 1)];
}

static inline void storeTranspositionEntry(uint64_t hash, int score, int depth, TTFlag flag)
{
    TTEntry *e = lookupTranspositionEntry(hash);
    // Chỉ ghi đè nếu depth mới sâu hơn hoặc cùng hash
    if (e->hash != hash || depth >= e->depth)
    {
        e->hash = hash;
        e->score = score;
        e->depth = static_cast<int8_t>(depth);
        e->flag = flag;
    }
}

// ============================================================
//  Line analysis
// ============================================================
struct LineInfo
{
    int count;
    int openEnds; // Số đầu hở thực sự (không tính biên bàn cờ)
};

struct MoveCandidate
{
    int row;
    int col;
    int score;
};

static inline bool isInBounds(int row, int col, int size)
{
    return row >= 0 && row < size && col >= 0 && col < size;
}

// openEnds chỉ tính khi ô kế tiếp nằm trong bàn VÀ trống
static LineInfo analyzeDirection(
    const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
    int size, int row, int col, int dirRow, int dirCol, int player)
{
    LineInfo res = {1, 0};

    // Hướng tiến
    int forwardRow = row + dirRow, forwardCol = col + dirCol;
    while (isInBounds(forwardRow, forwardCol, size) && board[forwardRow][forwardCol] == player)
    {
        res.count++;
        forwardRow += dirRow;
        forwardCol += dirCol;
    }
    // Đầu hở: phải trong bàn VÀ ô đó trống (không phải quân địch, không phải biên)
    if (isInBounds(forwardRow, forwardCol, size) && board[forwardRow][forwardCol] == CELL_EMPTY)
        res.openEnds++;

    // Hướng lùi
    int backwardRow = row - dirRow;
    int backwardCol = col - dirCol;
    while (isInBounds(backwardRow, backwardCol, size) && board[backwardRow][backwardCol] == player)
    {
        res.count++;
        backwardRow -= dirRow;
        backwardCol -= dirCol;
    }
    if (isInBounds(backwardRow, backwardCol, size) && board[backwardRow][backwardCol] == CELL_EMPTY)
        res.openEnds++;

    return res;
}

static int evaluateLine(const LineInfo &line, int winLen)
{
    if (line.count >= winLen)
        return 1000000;
    if (line.openEnds == 0)
        return 0;
    bool hasTwoOpenEnds = (line.openEnds == 2);
    switch (line.count)
    {
    case 4:
        return hasTwoOpenEnds ? 50000 : 10000;
    case 3:
        return hasTwoOpenEnds ? 5000 : 1000;
    case 2:
        return hasTwoOpenEnds ? 500 : 100;
    case 1:
        return hasTwoOpenEnds ? 50 : 10;
    default:
        return 0;
    }
}

static int scoreMove(
    const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
    int size, int winLen, int row, int col, int player)
{
    static const int dirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    int total = 0;
    for (int dirIdx = 0; dirIdx < 4; dirIdx++)
    {
        LineInfo line = analyzeDirection(board, size, row, col,
                                         dirs[dirIdx][0], dirs[dirIdx][1], player);
        total += evaluateLine(line, winLen);
    }
    return total;
}

// ============================================================
//  Candidate filter  (radius 2)
// ============================================================
static bool isCandidate(
    const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int row, int col)
{
    for (int deltaRow = -2; deltaRow <= 2; deltaRow++)
        for (int deltaCol = -2; deltaCol <= 2; deltaCol++)
        {
            int neighborRow = row + deltaRow, neighborCol = col + deltaCol;
            if (isInBounds(neighborRow, neighborCol, size) && board[neighborRow][neighborCol] != CELL_EMPTY)
                return true;
        }
    return false;
}

// ============================================================
//  Incremental board score
//  Trả về: p2_score - p1_score  (dương = P2 lợi thế)
// ============================================================
static int computeIncrementalScore(
    const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size, int winLen)
{
    int score = 0;
    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            if (board[row][col] == CELL_PLAYER2)
                score += scoreMove(board, size, winLen, row, col, CELL_PLAYER2);
            else if (board[row][col] == CELL_PLAYER1)
                score -= scoreMove(board, size, winLen, row, col, CELL_PLAYER1);
        }
    }
    return score;
}

// ============================================================
//  Quiescence check: thắng tức thì?
// ============================================================
static bool hasImmediateWin(
    const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
    int size, int winLen, int player)
{
    static const int dirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            if (board[row][col] != player)
                continue;
            for (int dirIdx = 0; dirIdx < 4; dirIdx++)
            {
                int consecutiveCount = 1;
                for (int step = 1; step < winLen; step++)
                {
                    int nextRow = row + dirs[dirIdx][0] * step, nextCol = col + dirs[dirIdx][1] * step;
                    if (isInBounds(nextRow, nextCol, size) && board[nextRow][nextCol] == player)
                        consecutiveCount++;
                    else
                        break;
                }
                if (consecutiveCount >= winLen)
                    return true;
            }
        }
    }
    return false;
}

// ============================================================
//  Alpha-Beta  với Transposition Table + Move Ordering
// ============================================================
static int alphaBetaSearch(
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
    int size, int winLen, int depth,
    int alpha, int beta, bool isMaximizing,
    uint64_t hash)
{
    // --- Transposition table lookup ---
    TTEntry *transEntry = lookupTranspositionEntry(hash);
    if (transEntry->hash == hash && transEntry->depth >= depth)
    {
        if (transEntry->flag == TT_EXACT)
            return transEntry->score;
        if (transEntry->flag == TT_LOWER && transEntry->score > alpha)
            alpha = transEntry->score;
        if (transEntry->flag == TT_UPPER && transEntry->score < beta)
            beta = transEntry->score;
        if (alpha >= beta)
            return transEntry->score;
    }

    // --- Terminal / depth check ---
    if (depth == 0)
        return computeIncrementalScore(board, size, winLen);

    // --- Kiểm tra thắng tức thì (quiescence nhẹ) ---
    if (hasImmediateWin(board, size, winLen, CELL_PLAYER2))
        return 900000 + depth;
    if (hasImmediateWin(board, size, winLen, CELL_PLAYER1))
        return -900000 - depth;

    // --- Sinh và sắp xếp nước đi ---
    vector<MoveCandidate> moves;
    moves.reserve(64);
    int currentPlayer = isMaximizing ? CELL_PLAYER2 : CELL_PLAYER1;
    int opponentPlayer = isMaximizing ? CELL_PLAYER1 : CELL_PLAYER2;

    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            if (board[row][col] != CELL_EMPTY || !isCandidate(board, size, row, col))
                continue;
            int attackScore = scoreMove(board, size, winLen, row, col, currentPlayer);
            int defenseScore = scoreMove(board, size, winLen, row, col, opponentPlayer);
            moves.push_back({row, col, attackScore + defenseScore});
        }
    }

    if (moves.empty())
        return computeIncrementalScore(board, size, winLen);

    sort(moves.begin(), moves.end(),
         [](const MoveCandidate &a, const MoveCandidate &b)
         { return a.score > b.score; });

    // Giới hạn nhánh để tránh nổ bộ nhớ ở depth cao
    static constexpr int MAX_BRANCHES = 12;
    if ((int)moves.size() > MAX_BRANCHES)
        moves.resize(MAX_BRANCHES);

    int originalAlpha = alpha;
    int best = isMaximizing ? INT_MIN : INT_MAX;

    for (const auto &m : moves)
    {
        uint64_t newHash = hash ^ zobristHashForCell(m.row, m.col, isMaximizing ? CELL_PLAYER2 : CELL_PLAYER1);
        board[m.row][m.col] = isMaximizing ? CELL_PLAYER2 : CELL_PLAYER1;

        int value = alphaBetaSearch(board, size, winLen, depth - 1, alpha, beta, !isMaximizing, newHash);

        board[m.row][m.col] = CELL_EMPTY;

        if (isMaximizing)
        {
            if (value > best)
                best = value;
            if (best > alpha)
                alpha = best;
        }
        else
        {
            if (value < best)
                best = value;
            if (best < beta)
                beta = best;
        }
        if (beta <= alpha)
            break; // Pruning
    }

    // --- Lưu vào transposition table ---
    TTFlag flag;
    if (best <= originalAlpha)
        flag = TT_UPPER;
    else if (best >= beta)
        flag = TT_LOWER;
    else
        flag = TT_EXACT;
    storeTranspositionEntry(hash, best, depth, flag);

    return best;
}

// ============================================================
//  Xây Zobrist hash từ trạng thái bàn cờ hiện tại
// ============================================================
static uint64_t buildHash(const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size)
{
    uint64_t h = 0;
    for (int row = 0; row < size; row++)
        for (int col = 0; col < size; col++)
            if (board[row][col] != CELL_EMPTY)
                h ^= zobristHashForCell(row, col, board[row][col]);
    return h;
}

// ============================================================
//  Fallback moves
// ============================================================
static void selectRandomMove(const PlayState *state, int &outRow, int &outCol)
{
    int size = state->boardSize;
    for (int attempts = 0; attempts < size * size; attempts++)
    {
        int randRow = rand() % size, randCol = rand() % size;
        if (state->board[randRow][randCol] == CELL_EMPTY)
        {
            outRow = randRow;
            outCol = randCol;
            return;
        }
    }
    for (int row = 0; row < size; row++)
        for (int col = 0; col < size; col++)
            if (state->board[row][col] == CELL_EMPTY)
            {
                outRow = row;
                outCol = col;
                return;
            }
}

static void selectSimpleMove(const PlayState *state, int &outRow, int &outCol)
{
    int bestScore = -1;
    int size = state->boardSize;
    int winLen = (state->gameMode == MODE_CARO) ? 5 : 3;

    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            if (state->board[row][col] != CELL_EMPTY || !isCandidate(state->board, size, row, col))
                continue;
            int attackScore = scoreMove(state->board, size, winLen, row, col, CELL_PLAYER2);
            int defenseScore = scoreMove(state->board, size, winLen, row, col, CELL_PLAYER1);
            int totalScore = attackScore + defenseScore * 2;
            if (totalScore > bestScore)
            {
                bestScore = totalScore;
                outRow = row;
                outCol = col;
            }
        }
    }
    if (bestScore == -1)
        selectRandomMove(state, outRow, outCol);
}

// ============================================================
//  Public entry point
// ============================================================
void calculateComputerMove(const PlayState *state, int difficulty, int &outRow, int &outCol)
{
    initializeZobrist();

    if (difficulty == 1)
    {
        selectRandomMove(state, outRow, outCol);
        return;
    }
    if (difficulty == 2)
    {
        selectSimpleMove(state, outRow, outCol);
        return;
    }

    // --- Difficulty 3: Alpha-Beta + Zobrist TT ---
    int size = state->boardSize;
    int winLen = (state->gameMode == MODE_CARO) ? 5 : 3;
    int depth = 5; // Tăng được thêm 1 depth nhờ TT + branching limit

    // Copy board sang virtualBoard
    int virtualBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE] = {0};
    for (int row = 0; row < MAX_BOARD_SIZE; row++)
        for (int col = 0; col < MAX_BOARD_SIZE; col++)
            virtualBoard[row][col] = state->board[row][col];

    uint64_t rootHash = buildHash(virtualBoard, size);

    // Sinh danh sách ứng cử viên ở root (không bị giới hạn MAX_BRANCHES)
    vector<MoveCandidate> moves;
    moves.reserve(64);
    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            if (virtualBoard[row][col] != CELL_EMPTY || !isCandidate(virtualBoard, size, row, col))
                continue;
            int attackScore = scoreMove(virtualBoard, size, winLen, row, col, CELL_PLAYER2);
            int defenseScore = scoreMove(virtualBoard, size, winLen, row, col, CELL_PLAYER1);
            moves.push_back({row, col, attackScore + defenseScore * 2});
        }
    }

    if (moves.empty())
    {
        selectRandomMove(state, outRow, outCol);
        return;
    }

    sort(moves.begin(), moves.end(),
         [](const MoveCandidate &a, const MoveCandidate &b)
         { return a.score > b.score; });

    // Kiểm tra nước thắng / chặn thua ngay lập tức (ưu tiên tuyệt đối)
    for (const auto &m : moves)
    {
        virtualBoard[m.row][m.col] = CELL_PLAYER2;
        if (hasImmediateWin(virtualBoard, size, winLen, CELL_PLAYER2))
        {
            outRow = m.row;
            outCol = m.col;
            virtualBoard[m.row][m.col] = CELL_EMPTY;
            return;
        }
        virtualBoard[m.row][m.col] = CELL_EMPTY;
    }
    for (const auto &m : moves)
    {
        virtualBoard[m.row][m.col] = CELL_PLAYER1;
        if (hasImmediateWin(virtualBoard, size, winLen, CELL_PLAYER1))
        {
            outRow = m.row;
            outCol = m.col;
            virtualBoard[m.row][m.col] = CELL_EMPTY;
            return;
        }
        virtualBoard[m.row][m.col] = CELL_EMPTY;
    }

    // Alpha-Beta từng nước ở root
    int bestScore = INT_MIN;
    outRow = moves[0].row;
    outCol = moves[0].col;

    for (const auto &m : moves)
    {
        uint64_t newHash = rootHash ^ zobristHashForCell(m.row, m.col, CELL_PLAYER2);
        virtualBoard[m.row][m.col] = CELL_PLAYER2;

        int score = alphaBetaSearch(virtualBoard, size, winLen,
                                    depth - 1, INT_MIN, INT_MAX, false, newHash);

        virtualBoard[m.row][m.col] = CELL_EMPTY;

        if (score > bestScore)
        {
            bestScore = score;
            outRow = m.row;
            outCol = m.col;
        }
    }
}