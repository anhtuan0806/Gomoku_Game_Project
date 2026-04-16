#ifndef _PLAY_STATE_H
#define _PLAY_STATE_H
#include "GameConstants.h"
#include "GameState.h"
#include <string>

#include <vector>
#include <utility>

using namespace std;
// Trạng thái ván đấu
enum MatchStatus
{
    MATCH_PLAYING,
    MATCH_PAUSED,
    MATCH_FINISHED,
    MATCH_SUMMARY
};

// Thông tin người chơi (mới)
struct PlayerInfo2
{
    wstring name;
    string avatarPath;
    char piece;
    int totalWins = 0;       // Thêm = 0
    int matchWins = 0;       // Thêm = 0
    int movesCount = 0;      // Thêm = 0
    float maxTurnTime = 0.0f;
    float totalTimePossessed = 0.0f;
};

// Trạng thái ván đấu
struct PlayState
{
    PlayMode gameMode;
    MatchType matchType;

    PlayerInfo2 p1;
    PlayerInfo2 p2;

    bool isP1Turn;

    int countdownTime;
    int timeRemaining;

    int boardSize;
    // Ma trận lưu giá trị: CELL_EMPTY, CELL_PLAYER1, hoặc CELL_PLAYER2
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];

    // Tọa độ con trỏ hiện tại trên bàn cờ
    int cursorRow;
    int cursorCol;
    
    // Lưu tọa độ vừa đánh và chuỗi các ô tạo nên đường thắng
    int lastMoveRow;
    int lastMoveCol;
    std::vector<std::pair<int, int>> winningCells;
    
    // Lịch sử bước đi
    std::vector<std::pair<int, int>> matchHistory;
    std::vector<std::pair<int, int>> redoStack;

    MatchStatus status;
    int winner; // 0: Hòa, 1: P1, 2: P2, -1: Đang chơi

    int difficulty;       // 1: Dễ, 2: Trung bình, 3: Khó
    int targetScore;      // 1 (Bo1), 2 (Bo3 - thắng 2 ván), 3 (Bo5 - thắng 3 ván)

    // Quản lý thời gian
    float matchDuration;  // Tổng thời gian PVE (giây)
    int p1TotalTimeLeft;
    int p2TotalTimeLeft;
    bool isMatchTimed;    // Có bật giới hạn thời gian cả trận không

    std::wstring saveName;      // Tên hiển thị của bản lưu
    std::wstring saveTimestamp; // Thời gian lưu (Ngày/Giờ)
};

#endif // _PLAY_STATE_H