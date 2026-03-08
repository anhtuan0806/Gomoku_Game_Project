#pragma once

// Quản lý trạng thái màn hình hiện tại của ứng dụng
enum ScreenState {
    SCREEN_MENU,
    SCREEN_ABOUT,
    SCREEN_GUIDE,
    SCREEN_PLAY,
    SCREEN_LOAD_GAME,
    SCREEN_SETTING,
    SCREEN_EXIT
};

// Loại trò chơi
enum PlayMode {
    MODE_CARO,
    MODE_TIC_TAC_TOE
};

// Chế độ người chơi
enum MatchType {
    MATCH_PVP, // Người vs Người
    MATCH_PVE  // Người vs Máy
};