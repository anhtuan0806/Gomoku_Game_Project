#include "BotAI.h"
#include "GameRules.h"
#include <cstdlib>
#include <climits>
#include <cstring>
#include <vector>
#include <algorithm>
#include <random>


/** @file BotAI.cpp
 *  @brief Triển khai AI Bot: Zobrist hashing, Transposition Table và Alpha-Beta search.
 */

/** Zobrist table nội bộ: [row][col][pieceIndex] (pieceIndex: 0 = P1, 1 = P2) */
static std::uint64_t sZobristTable[MAX_BOARD_SIZE][MAX_BOARD_SIZE][2];
static bool sIsZobristInitialized = false;

/**
 * @brief Khởi tạo bảng Zobrist (sử dụng rng cố định để kết quả có thể tái tạo).
 * @note Hàm an toàn khi gọi nhiều lần (idempotent).
 */
static void initializeZobrist()
{
    if (sIsZobristInitialized)
        return;
    std::mt19937_64 rng(0xDEADBEEFCAFEBABEULL);
    for (int row = 0; row < MAX_BOARD_SIZE; row++)
        for (int col = 0; col < MAX_BOARD_SIZE; col++)
            for (int pieceIdx = 0; pieceIdx < 2; pieceIdx++)
                sZobristTable[row][col][pieceIdx] = rng();
    sIsZobristInitialized = true;
}

/**
 * @brief Lấy giá trị Zobrist cho ô (row,col) và piece (CELL_PLAYER1/2).
 */
static inline std::uint64_t zobristHashForCell(int row, int col, int piece)
{
    // piece: CELL_PLAYER1=1 → idx 0, CELL_PLAYER2=2 → idx 1
    return sZobristTable[row][col][piece - 1];
}

/** Transposition table nội bộ. */
static TTEntry sTranspositionTable[sTranspositionTableSize];

/**
 * @brief Xóa toàn bộ Transposition Table (zero-fill).
 * @note Gọi khi bắt đầu ván mới để tránh dùng entry cũ không phù hợp.
 */
void clearTranspositionTable()
{
    std::memset(sTranspositionTable, 0, sizeof(sTranspositionTable));
}

/**
 * @brief Tra cứu entry trong TT theo hash (chỉ trả địa chỉ bucket).
 * @return Con trỏ tới bucket tương ứng.
 */
static inline TTEntry *lookupTranspositionEntry(std::uint64_t hash)
{
    return &sTranspositionTable[hash & (sTranspositionTableSize - 1)];
}

/**
 * @brief Lưu một entry vào Transposition Table theo bucket hash.
 * @param hash Zobrist hash của trạng thái.
 * @param score Giá trị đánh giá.
 * @param depth Độ sâu của giá trị.
 * @param flag Loại entry (exact/lower/upper).
 *
 * @note Chỉ ghi đè khi bucket rỗng hoặc depth mới >= depth cũ để giữ giá trị sâu hơn.
 */
static inline void storeTranspositionEntry(std::uint64_t hash, int score, int depth, TTFlag flag)
{
    TTEntry *e = lookupTranspositionEntry(hash);
    // Chỉ ghi đè nếu depth mới sâu hơn hoặc cùng hash
    if (e->hash != hash || depth >= e->depth)
    {
        e->hash = hash;
        e->score = score;
        e->depth = static_cast<std::int8_t>(depth);
        e->flag = flag;
    }
}

/**
 * @brief Thông tin dọc (line) dùng để đánh giá chuỗi liên tiếp của quân cờ.
 */
struct LineInfo
{
    int count;    /**< Số quân liên tiếp tính cả ô hiện tại */
    int openEnds; /**< Số đầu hở thực sự (0,1,2) */
};

/**
 * @brief Candidate move với điểm số tạm tính dùng cho ordering.
 */
struct MoveCandidate
{
    int row;
    int col;
    int score;
};

/**
 * @brief Kiểm tra tọa độ nằm trong bàn.
 */
static inline bool isInBounds(int row, int col, int size)
{
    return row >= 0 && row < size && col >= 0 && col < size;
}

/**
 * @brief Phân tích một hướng (dirRow,dirCol) bắt đầu từ ô (row,col) cho `player`.
 * @return LineInfo chứa số quân liên tiếp và số đầu hở.
 * @note `openEnds` chỉ tính khi ô kế tiếp nằm trong bàn VÀ trống.
 */
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

/**
 * @brief Chuyển `LineInfo` thành một điểm số heuristic.
 * @param line LineInfo phân tích được.
 * @param winLen Số lượng liên tiếp cần thắng.
 * @return Điểm số heuristic (số lớn nếu thắng ngay).
 */
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

/**
 * @brief Tính điểm tấn công/phòng thủ cho một nước đi (row,col) cho `player`.
 */
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

/**
 * @brief Kiểm tra xem ô (row,col) có phải là candidate (gần một quân nào đó trong radius 2).
 */
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

/**
 * @brief Tính điểm tổng cho bàn hiện tại (p2_score - p1_score).
 * @return Giá trị dương nếu P2 có lợi thế.
 */
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

/**
 * @brief Kiểm tra nhanh xem có chuỗi thắng tức thì cho `player` hay không.
 * @note Dùng làm quiescence/fast terminal check trong search.
 */
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

/**
 * @brief Kiểm tra thắng tức thì CHỈ quanh ô (lastRow, lastCol) vừa đặt.
 *
 * Giảm từ O(N²) xuống O(winLen×4) cho mỗi lần gọi.
 * Điều kiện: ô (lastRow, lastCol) đã chứa quân `player`.
 */
static bool hasImmediateWinAroundMove(
    const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
    int size, int winLen, int lastRow, int lastCol, int player)
{
    static const int dirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int dirIdx = 0; dirIdx < 4; dirIdx++)
    {
        int consecutiveCount = 1;
        // Tiến
        for (int step = 1; step < winLen; step++)
        {
            int nextRow = lastRow + dirs[dirIdx][0] * step;
            int nextCol = lastCol + dirs[dirIdx][1] * step;
            if (isInBounds(nextRow, nextCol, size) && board[nextRow][nextCol] == player)
                consecutiveCount++;
            else
                break;
        }
        // Lùi
        for (int step = 1; step < winLen; step++)
        {
            int nextRow = lastRow - dirs[dirIdx][0] * step;
            int nextCol = lastCol - dirs[dirIdx][1] * step;
            if (isInBounds(nextRow, nextCol, size) && board[nextRow][nextCol] == player)
                consecutiveCount++;
            else
                break;
        }
        if (consecutiveCount >= winLen)
            return true;
    }
    return false;
}

/**
 * @brief Alpha-Beta search lõi, sử dụng Transposition Table và move ordering.
 *
 * @param board Bảng chơi (mutable per search frame).
 * @param size Kích thước bàn.
 * @param winLen Số quân cần liên tiếp để thắng.
 * @param depth Độ sâu còn lại.
 * @param alpha Alpha bound.
 * @param beta Beta bound.
 * @param isMaximizing True nếu node hiện tại là maximizing (P2).
 * @param hash Zobrist hash hiện tại của `board`.
 * @return Giá trị đánh giá (int).
 */
static int alphaBetaSearch(
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
    int size, int winLen, int depth,
    int alpha, int beta, bool isMaximizing,
    std::uint64_t hash,
    int lastRow, int lastCol)
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

    // --- Kiểm tra thắng tức thì quanh nước đi cuối (O(winLen) thay vì O(N²)) ---
    if (lastRow >= 0 && lastCol >= 0)
    {
        int lastPlayer = board[lastRow][lastCol];
        if (lastPlayer != CELL_EMPTY &&
            hasImmediateWinAroundMove(board, size, winLen, lastRow, lastCol, lastPlayer))
        {
            return (lastPlayer == CELL_PLAYER2) ? (900000 + depth) : (-900000 - depth);
        }
    }

    // --- Sinh và sắp xếp nước đi ---
    std::vector<MoveCandidate> moves;
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

    std::sort(moves.begin(), moves.end(),
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
        std::uint64_t newHash = hash ^ zobristHashForCell(m.row, m.col, isMaximizing ? CELL_PLAYER2 : CELL_PLAYER1);
        board[m.row][m.col] = isMaximizing ? CELL_PLAYER2 : CELL_PLAYER1;

        int value = alphaBetaSearch(board, size, winLen, depth - 1, alpha, beta, !isMaximizing, newHash, m.row, m.col);

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

/**
 * @brief Xây Zobrist hash từ trạng thái bàn cờ hiện tại.
 */
static std::uint64_t buildHash(const int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int size)
{
    std::uint64_t h = 0;
    for (int row = 0; row < size; row++)
        for (int col = 0; col < size; col++)
            if (board[row][col] != CELL_EMPTY)
                h ^= zobristHashForCell(row, col, board[row][col]);
    return h;
}

/**
 * @brief Chọn nước đi ngẫu nhiên hợp lệ (fallback).
 */
static void selectRandomMove(const PlayState *state, int &outRow, int &outCol)
{
    int size = state->boardSize;
    for (int attempts = 0; attempts < size * size; attempts++)
    {
        int randRow = std::rand() % size, randCol = std::rand() % size;
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

/**
 * @brief Chọn nước đi đơn giản dựa trên heuristic tấn công/phòng thủ.
 */
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

/**
 * @brief Tính toán nước đi cho máy dựa trên `difficulty` và trạng thái hiện tại.
 *
 * @param state Trạng thái ván chơi hiện thời.
 * @param difficulty 1..3 (1 Random, 2 heuristic, 3 alpha-beta).
 * @param outRow Tham chiếu trả về hàng nước đi.
 * @param outCol Tham chiếu trả về cột nước đi.
 */
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

    std::uint64_t rootHash = buildHash(virtualBoard, size);

    // Sinh danh sách ứng cử viên ở root (không bị giới hạn MAX_BRANCHES)
    std::vector<MoveCandidate> moves;
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

    std::sort(moves.begin(), moves.end(),
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
        std::uint64_t newHash = rootHash ^ zobristHashForCell(m.row, m.col, CELL_PLAYER2);
        virtualBoard[m.row][m.col] = CELL_PLAYER2;

        int score = alphaBetaSearch(virtualBoard, size, winLen,
                                    depth - 1, INT_MIN, INT_MAX, false, newHash, m.row, m.col);

        virtualBoard[m.row][m.col] = CELL_EMPTY;

        if (score > bestScore)
        {
            bestScore = score;
            outRow = m.row;
            outCol = m.col;
        }
    }
}