#include "ConfigLoader.h"
#include "../ApplicationTypes/GameConfig.h"
#include <fstream>
#include <iostream>

/** @file ConfigLoader.cpp
 *  @brief Triển khai tải/ghi cấu hình `GameConfig` theo định dạng nhị phân.
 *
 *  Lưu ý: file lưu ở dạng POD của `GameConfig` để đọc/ghi nhanh. Nếu file không tồn tại
 *  hoặc kích thước khác với `sizeof(GameConfig)` thì hàm sẽ ghi cấu hình mặc định.
 */

// (g_Config is now defined in ApplicationTypes/GameConfig.cpp)

/** @brief Thiết lập giá trị cấu hình mặc định. */
static void SetDefaultConfig(GameConfig *config)
{
    config->isBgmEnabled = true;
    config->bgmVolume = 50;
    config->isSfxEnabled = true;
    config->sfxVolume = 50;
    config->currentLang = APP_LANG_VI;
    config->isVisualEffectsEnabled = true;
    config->fpsLimit = 60;
}

/** @brief Kiểm tra tính hợp lệ cơ bản của `GameConfig`.
 *  @return true nếu các trường nằm trong khoảng hợp lệ.
 */
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

/**
 * @brief Tải cấu hình từ `filepath` vào `config`.
 * @details Nếu file không tồn tại hoặc kích thước không khớp, lưu cấu hình mặc định trở lại file.
 */
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

/** @brief Ghi cấu hình `config` ra `filepath` (ghi nhị phân). */
void SaveConfig(const GameConfig *config, const std::string &filepath)
{
    std::ofstream file(filepath, std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char *>(config), sizeof(GameConfig));
        file.close();
    }
}