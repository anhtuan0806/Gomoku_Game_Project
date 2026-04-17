#ifndef _GAME_STATE_H
#define _GAME_STATE_H

enum ScreenState
{
    SCREEN_MENU,
    SCREEN_MATCH_CONFIG,
    SCREEN_PLAY,
    SCREEN_LOAD_GAME,
    SCREEN_SETTING,
    SCREEN_EXIT,
    SCREEN_GUIDE,
    SCREEN_ABOUT
};

enum PlayMode
{
    MODE_CARO,
    MODE_TIC_TAC_TOE
};

enum MatchType
{
    MATCH_PVP,
    MATCH_PVE
};

#endif // _GAME_STATE_H