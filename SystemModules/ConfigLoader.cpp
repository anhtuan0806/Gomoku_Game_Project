#include "ConfigLoader.h"
#include <fstream>
#include <string>

void LoadConfig(GameConfig* config, const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        // Cài đặt mặc định nếu không tìm thấy tệp
        config->isBgmEnabled = true;
        config->bgmVolume = 100;
        config->isSfxEnabled = true;
        config->sfxVolume = 100;
        config->currentLang = APP_LANG_VIETNAMESE;
        config->currentTheme = THEME_CLASSIC;
        return;
    }

    std::string key;
    int value;
    while (file >> key >> value) {
        if (key == "isBgmEnabled") config->isBgmEnabled = (value != 0);
        else if (key == "bgmVolume") config->bgmVolume = value;
        else if (key == "isSfxEnabled") config->isSfxEnabled = (value != 0);
        else if (key == "sfxVolume") config->sfxVolume = value;
        else if (key == "currentLang") config->currentLang = (Language)value;
        else if (key == "currentTheme") config->currentTheme = (BoardTheme)value;
    }
    file.close();
}

void SaveConfig(const GameConfig* config, const char* filepath) {
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << "isBgmEnabled " << config->isBgmEnabled << "\n";
        file << "bgmVolume " << (int)config->bgmVolume << "\n";
        file << "isSfxEnabled " << config->isSfxEnabled << "\n";
        file << "sfxVolume " << (int)config->sfxVolume << "\n";
        file << "currentLang " << config->currentLang << "\n";
        file << "currentTheme " << config->currentTheme << "\n";
        file.close();
    }
}