#ifndef _GAME_CONFIG_H
#define _GAME_CONFIG_H

enum Language {
    APP_LANG_VI,
    APP_LANG_EN
};

enum BoardTheme {
    THEME_CLASSIC,
    THEME_NEON,
    THEME_RETRO
};

struct GameConfig {
    bool isBgmEnabled;
    int bgmVolume;      
    bool isSfxEnabled;
    int sfxVolume;
    Language currentLang;
    BoardTheme currentTheme;
};

#endif