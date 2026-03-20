#ifndef _PLAY_STATE_H
#define _PLAY_STATE_H
#include "GameConstants.h"
#include "GameState.h"

enum MatchStatus {
    MATCH_PLAYING,
    MATCH_PAUSED,
    MATCH_FINISHED
};

struct PlayerInfo {
    char name[50];
    int avatarID;    
    char piece;        // 'X' hoặc 'O'
    int score;
    int movesCount;
};

struct PlayState {
    PlayMode gameMode;
    MatchType matchType;

    PlayerInfo p1;
    PlayerInfo p2;

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