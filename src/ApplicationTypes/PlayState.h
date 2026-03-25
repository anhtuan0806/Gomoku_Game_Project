#ifndef _PLAY_STATE_H
#define _PLAY_STATE_H
#include "GameConstants.h"
#include "GameState.h"
#include <string>

using std::string, std::wstring;

// Trạng thái ván đấu
enum MatchStatus
{
    MATCH_PLAYING,
    MATCH_PAUSED,
    MATCH_FINISHED
};

// Thông tin người chơi (cũ)
struct PlayerInfo
{
    char name[50];
    int avatarID;
    char piece;        // 'X' hoặc 'O'
    int score;
    int movesCount;
};

// Thông tin người chơi (mới)
struct PlayerInfo2
{
    wstring name;
    string avatarPath;
    char piece;          // 'X' hoặc 'O'      
    int totalWins;
    int movesCount;
    float maxTurnTime;
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

    MatchStatus status;
    int winner; // 0: Hòa, 1: P1, 2: P2, -1: Đang chơi
};

#endif // _PLAY_STATE_H