#include "ConfigLoader.h"
#include <fstream>

static void SetDefaultConfig(GameConfig* config) {
    config->isBgmEnabled = true;
    config->bgmVolume = 50;
    config->isSfxEnabled = true;
    config->sfxVolume = 50;
    config->currentLang = APP_LANG_VI;
    config->currentTheme = THEME_CLASSIC;
}

void LoadConfig(GameConfig* config, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        SetDefaultConfig(config); // Không có file thì dùng mặc định
        return;
    }
    file.read(reinterpret_cast<char*>(config), sizeof(GameConfig));
    file.close();
}

void SaveConfig(const GameConfig* config, const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(config), sizeof(GameConfig));
        file.close();
    }
}