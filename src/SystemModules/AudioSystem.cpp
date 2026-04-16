#include "AudioSystem.h"
#include "../ApplicationTypes/GameConfig.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

extern GameConfig g_Config;

void PlayBGM(const std::string& filepath) {
    if (!g_Config.isBgmEnabled) return;
    StopBGM();
    std::string cmd = "open \"" + filepath + "\" type mpegvideo alias bgm";
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
    UpdateBGMVolume();
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
}

void StopBGM() {
    mciSendStringA("close bgm", NULL, 0, NULL);
}

void UpdateBGMVolume() {
    if (g_Config.bgmVolume <= 0) {
        // Tắt hẳn luồng phát âm thanh nếu Volume = 0
        mciSendStringA("setaudio bgm off", NULL, 0, NULL);
    }
    else {
        // Bật lại luồng âm thanh và set Volume
        mciSendStringA("setaudio bgm on", NULL, 0, NULL);
        int vol = g_Config.bgmVolume * 10;
        std::string volCmd = "setaudio bgm volume to " + std::to_string(vol);
        mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
    }
}

void PlaySFX(const std::string& filepath, const std::string& alias) {
    // Kỷ luật sắt: Nếu tắt SFX hoặc Âm lượng = 0 thì KHÔNG LÀM GÌ CẢ
    if (!g_Config.isSfxEnabled || g_Config.sfxVolume <= 0) return;

    std::string closeCmd = "close " + alias;
    mciSendStringA(closeCmd.c_str(), NULL, 0, NULL);

    std::string openCmd = "open \"" + filepath + "\" type mpegvideo alias " + alias;
    mciSendStringA(openCmd.c_str(), NULL, 0, NULL);

    int vol = g_Config.sfxVolume * 10;
    std::string volCmd = "setaudio " + alias + " volume to " + std::to_string(vol);
    mciSendStringA(volCmd.c_str(), NULL, 0, NULL);

    std::string playCmd = "play " + alias;
    mciSendStringA(playCmd.c_str(), NULL, 0, NULL);
}

void StopSFX(const std::string& alias) {
    std::string closeCmd = "close " + alias;
    mciSendStringA(closeCmd.c_str(), NULL, 0, NULL);
}