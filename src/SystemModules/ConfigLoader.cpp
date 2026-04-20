#include "ConfigLoader.h"
#include <fstream>

static void SetDefaultConfig(GameConfig *config)
{
    config->isBgmEnabled = true;
    config->bgmVolume = 50;
    config->isSfxEnabled = true;
    config->sfxVolume = 50;
    config->currentLang = APP_LANG_VI;
}

static bool IsValidConfig(const GameConfig &config)
{
    if (config.bgmVolume < 0 || config.bgmVolume > 100)
        return false;
    if (config.sfxVolume < 0 || config.sfxVolume > 100)
        return false;
    if (config.currentLang != APP_LANG_VI && config.currentLang != APP_LANG_EN)
        return false;
    return true;
}

void LoadConfig(GameConfig *config, const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        SetDefaultConfig(config); // Không có file thì dùng mặc định
        return;
    }
    auto size = file.tellg();
    if (size != static_cast<std::streamoff>(sizeof(GameConfig)))
    {
        file.close();
        SetDefaultConfig(config);
        SaveConfig(config, filepath);
        return;
    }
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(config), sizeof(GameConfig));
    file.close();

    if (!IsValidConfig(*config))
    {
        SetDefaultConfig(config);
        SaveConfig(config, filepath);
    }
}

void SaveConfig(const GameConfig *config, const std::string &filepath)
{
    std::ofstream file(filepath, std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char *>(config), sizeof(GameConfig));
        file.close();
    }
}