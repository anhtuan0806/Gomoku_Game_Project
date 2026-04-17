#include "AudioSystem.h"
#include "../ApplicationTypes/GameConfig.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

extern GameConfig g_Config;

// Hàm hỗ trợ nạp âm thanh - giúp code sạch hơn
void PreLoad(const std::string& path, const std::string& alias) {
    std::string cmd = "open \"" + path + "\" type mpegvideo alias " + alias;
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}

void InitAudioSystem() {
    // Nạp sẵn tất cả SFX hay dùng vào RAM
    PreLoad("Asset/audio/move.wav", "sfx_move");
    PreLoad("Asset/audio/select.wav", "sfx_select");
    PreLoad("Asset/audio/DatCo.wav", "sfx_place");
    PreLoad("Asset/audio/Tiengcoi.wav", "sfx_whistle");
    PreLoad("Asset/audio/success.wav", "sfx_success");
    PreLoad("Asset/audio/siuuu.wav", "sfx_siu");
    PreLoad("Asset/audio/TiengKhanGia_Het_Tran.wav", "sfx_crowd");
}

void PlaySFX(const std::string& alias) {
    if (!g_Config.isSfxEnabled || g_Config.sfxVolume <= 0) return;

    // Không dùng open/close ở đây nữa -> Tốc độ phản hồi cực nhanh
    // Chỉ cần tua về đầu và phát
    std::string volCmd = "setaudio " + alias + " volume to " + std::to_string(g_Config.sfxVolume * 10);
    mciSendStringA(volCmd.c_str(), NULL, 0, NULL);

    std::string playCmd = "play " + alias + " from 0";
    mciSendStringA(playCmd.c_str(), NULL, 0, NULL);
}

void StopSFX(const std::string& alias) {
    std::string cmd = "stop " + alias;
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}

void PlayBGM(const std::string& filepath) {
    if (!g_Config.isBgmEnabled) return;
    StopBGM();
    // BGM thường dài nên vẫn dùng cơ chế open khi cần để tiết kiệm RAM
    std::string cmd = "open \"" + filepath + "\" type mpegvideo alias bgm";
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
    UpdateBGMVolume();
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
}

void StopBGM() {
    mciSendStringA("close bgm", NULL, 0, NULL);
}

void UpdateBGMVolume() {
    int vol = g_Config.bgmVolume * 10;
    std::string volCmd = "setaudio bgm volume to " + std::to_string(vol);
    mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
}

void ShutdownAudioSystem() {
    // Giải phóng tất cả alias đã nạp
    mciSendStringA("close all", NULL, 0, NULL);
}