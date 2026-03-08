#pragma once
#include <cstdint>

// Lựa chọn ngôn ngữ
enum Language {
    APP_LANG_VIETNAMESE,
    APP_LANG_ENGLISH
};

// Lựa chọn chủ đề bàn cờ 
enum BoardTheme { // tam thời 
    THEME_CLASSIC,
    THEME_NEON,
    THEME_RETRO
};

// Cấu trúc lưu trữ toàn bộ cài đặt game
struct GameConfig {
    bool isBgmEnabled;      // Bật/tắt nhạc nền
    uint8_t bgmVolume;          // Âm lượng nhạc nền (0 - 100)

    bool isSfxEnabled;      // Bật/tắt hiệu ứng âm thanh (tiếng đặt cờ, click...)
    uint8_t sfxVolume;          // Âm lượng hiệu ứng (0 - 100)

    Language currentLang;   // Ngôn ngữ hiện tại
    BoardTheme currentTheme;// Chủ đề giao diện và âm thanh hiện tại
};