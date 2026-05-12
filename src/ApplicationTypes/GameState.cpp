#include "GameState.h"
#include "GameConfig.h"
#include "PlayState.h"

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
    // Chỉ các screen có animation liên tục (không phụ thuộc input) mới cần redraw mỗi frame.
    // Các screen tĩnh (Settings, Config, Load, Guide, About, Pause) chỉ redraw khi có input/dirty rect.
    if (!g_Config.isVisualEffectsEnabled)
    {
        return (screen == SCREEN_PLAY && g_PlayState.status != MATCH_PAUSED);
    }

    switch (screen)
    {
    case SCREEN_MENU:         // Trophy float, camera flash, clouds, balloons
    case SCREEN_MATCH_CONFIG: // Avatar glow, selected item pulse
        return true;
    case SCREEN_PLAY:         // Cursor pulse, timer, match duration counter
        return (g_PlayState.status != MATCH_PAUSED);
    default:
        return false;          // Settings, Load, Guide, About, Pause — redraw on input only
    }
}
