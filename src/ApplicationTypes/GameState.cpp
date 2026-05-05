#include "GameState.h"
#include "GameConfig.h"

// --- Globals ---
ScreenState g_CurrentScreen = SCREEN_MENU;

int g_ConfigSelected = 0;
int g_MenuSelected = 0;
int g_LoadSelected = 0;
int g_SettingSelected = 0;
std::wstring g_LoadStatus = L"";
int g_GuildPage = 0;

bool ShouldAnimateScreen(ScreenState screen)
{
    // Tắt hiệu ứng hình ảnh: chỉ animate ở SCREEN_PLAY (cursor pulse, last move)
    if (!g_Config.isVisualEffectsEnabled)
    {
        return (screen == SCREEN_PLAY);
    }

    switch (screen)
    {
    case SCREEN_MENU:
    case SCREEN_PLAY:
    case SCREEN_SETTING:
    case SCREEN_MATCH_CONFIG:
    case SCREEN_LOAD_GAME:
    case SCREEN_GUIDE:
    case SCREEN_ABOUT:
        return true;
    default:
        return false;
    }
}
