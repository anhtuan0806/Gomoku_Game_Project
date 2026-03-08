#pragma once
#include <cstdint>
#include "GameState.h"

// Kích thước tối đa cho mảng tĩnh
#define MAX_BOARD_SIZE 20 

// Trạng thái của trận đấu hiện tại
enum MatchStatus {
    MATCH_PLAYING,
    MATCH_PAUSED,
    MATCH_FINISHED
};

// Danh sách các nhân vật có thể chọn làm Avatar
enum AvatarType { // tạm thời
    AVATAR_HERO,    
    AVATAR_WARRIOR,  
    AVATAR_MAGE,      
    AVATAR_ROBOT,     
    AVATAR_MONSTER   
};

// Cấu trúc một điểm trên bàn cờ
struct Point2D {
    uint8_t x; // Tọa độ hiển thị trên màn hình Console (cột)
    uint8_t y; // Tọa độ hiển thị trên màn hình Console (dòng)
    int8_t c; // Giá trị đánh dấu: 0 (Chưa đánh), -1 (Người 1), 1 (Người 2) 
};

// Thông tin chi tiết của một người chơi
struct PlayerInfo {
    char name[50];         // Tên người chơi
    AvatarType avatar;     // Mã nhân vật đại diện (Dùng để vẽ hình ASCII lên UI)
    char pieceSymbol;      // Ký tự quân cờ khi đánh xuống bàn (VD: 'X' hoặc 'O') 
    uint8_t score;             // Điểm số hiện tại (số ván thắng)
    uint8_t movesCount;        // Số nước đã đi trong ván
};

// Toàn bộ dữ liệu của một ván game
struct PlayState {
    PlayMode gameMode;         // Thể loại game: Caro hay Tic-Tac-Toe
    MatchType matchType;       // Chế độ chơi: PvP (Người vs Người) hay PvE (Người vs Máy)

    PlayerInfo player1;        // Dữ liệu người chơi 1
    PlayerInfo player2;        // Dữ liệu người chơi 2 (hoặc của Máy/Bot)

    bool isPlayer1Turn;        // Cờ xác định lượt: true = lượt Người 1, false = lượt Người 2/Máy
    uint8_t countdownTime;         // Cài đặt thời gian đếm ngược tối đa cho 1 lượt đi (ví dụ: 15s, 30s)
    uint8_t timeRemaining;         // Thời gian còn lại của lượt đi hiện tại (giây), sẽ giảm dần

    uint8_t boardSize;             // Kích thước bàn cờ thực tế đang dùng (3 cho Tic-Tac-Toe, 12 cho Caro) 
    Point2D board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]; // Ma trận lưu trữ toàn bộ các ô trên bàn cờ (chuyển sang mảng tĩnh cho tối ưu đi =>)

    Point2D cursor;            // Tọa độ con trỏ (x, y) để vẽ highlight nhấp nháy trên Console khi bấm W, A, S, D 

    MatchStatus status;        // Trạng thái trận đấu: Đang diễn ra, Tạm dừng, hay Đã kết thúc
    int8_t winner;                // Kết quả: 0 (Hòa), -1 (Người 1 thắng), 1 (Người 2 thắng), 2 (Chưa phân thắng bại) 
};