#ifndef _GAME_CONFIG_H
#define _GAME_CONFIG_H

enum Language
{
    APP_LANG_VI,
    APP_LANG_EN
};

struct GameConfig
{
    bool isBgmEnabled;
    int bgmVolume;
    bool isSfxEnabled;
    int sfxVolume;
    Language currentLang;
};

#endif