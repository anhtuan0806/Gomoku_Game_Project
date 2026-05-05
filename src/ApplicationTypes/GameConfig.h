#ifndef _GAME_CONFIG_H
#define _GAME_CONFIG_H

/** @file GameConfig.h
 *  @brief Cấu hình toàn cục ứng dụng (âm thanh, ngôn ngữ...).
 */

/** @brief Ngôn ngữ ứng dụng được hỗ trợ. */
enum Language
{
    APP_LANG_VI, /**< Tiếng Việt */
    APP_LANG_EN  /**< Tiếng Anh */
};

/** @brief Cấu trúc lưu các tuỳ chọn cấu hình ứng dụng. */
struct GameConfig
{
    bool isBgmEnabled;             /**< Bật/Tắt nhạc nền */
    int bgmVolume;                 /**< Mức âm lượng nhạc nền (0..100) */
    bool isSfxEnabled;             /**< Bật/Tắt âm hiệu ứng */
    int sfxVolume;                 /**< Mức âm lượng SFX (0..100) */
    Language currentLang;          /**< Ngôn ngữ hiện tại */
    bool isVisualEffectsEnabled;   /**< Hiệu ứng hình ảnh: clouds, wind, balloons, flash */
    int fpsLimit;                  /**< Giới hạn FPS mục tiêu (30 hoặc 60) */
};

// --- Globals (Owned by GameConfig) ---
extern GameConfig g_Config;

#endif