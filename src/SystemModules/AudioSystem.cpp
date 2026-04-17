#include "AudioSystem.h"
#include "../ApplicationTypes/GameConfig.h"
#include <windows.h>
#include <mmsystem.h>
#include <map>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#pragma comment(lib, "winmm.lib")

extern GameConfig g_Config;

// Thống nhất hệ thống SFX Async
struct SFXRequest {
    std::string alias;
    int volume;
};

static std::queue<SFXRequest> g_SFXQueue;
static std::mutex g_SFXMutex;
static std::condition_variable g_SFXCond;
static std::thread g_SFXThread;
static std::atomic<bool> g_SFXRunning{ false };
static std::map<std::string, int> g_LastSFXVol;
static std::map<std::string, ULONGLONG> g_LastSFXTime;

// Worker thread xử lý lệnh mci từ luồng nền
void SFXWorker() {
    while (g_SFXRunning) {
        SFXRequest req;
        {
            std::unique_lock<std::mutex> lock(g_SFXMutex);
            g_SFXCond.wait(lock, [] { return !g_SFXQueue.empty() || !g_SFXRunning; });
            if (!g_SFXRunning && g_SFXQueue.empty()) break;
            req = g_SFXQueue.front();
            g_SFXQueue.pop();
        }

        // Thực thi lệnh mci trên luồng nền
        if (g_LastSFXVol[req.alias] != req.volume) {
            std::string volCmd = "setaudio " + req.alias + " volume to " + std::to_string(req.volume);
            mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
            g_LastSFXVol[req.alias] = req.volume;
        }

        std::string playCmd = "play " + req.alias + " from 0";
        mciSendStringA(playCmd.c_str(), NULL, 0, NULL);
    }
}

void PreLoad(const std::string& path, const std::string& alias) {
    std::string cmd = "open \"" + path + "\" type mpegvideo alias " + alias;
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}

void InitAudioSystem() {
    // Khởi tạo luồng xử lý SFX
    if (!g_SFXRunning) {
        g_SFXRunning = true;
        g_SFXThread = std::thread(SFXWorker);
    }

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

    // Cooldown 50ms ngăn việc spam hàng đợi quá mức
    ULONGLONG now = GetTickCount64();
    if (now - g_LastSFXTime[alias] < 50ULL) return;
    g_LastSFXTime[alias] = now;

    int targetVol = g_Config.sfxVolume * 10;
    
    // Đẩy yêu cầu vào hàng đợi cho luồng nền xử lý
    {
        std::lock_guard<std::mutex> lock(g_SFXMutex);
        g_SFXQueue.push({ alias, targetVol });
    }
    g_SFXCond.notify_one();
}

void StopSFX(const std::string& alias) {
    std::string cmd = "stop " + alias;
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}

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
    static int lastBgmVol = -1;
    int vol = g_Config.bgmVolume * 10;
    if (lastBgmVol == vol) return;

    std::string volCmd = "setaudio bgm volume to " + std::to_string(vol);
    mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
    lastBgmVol = vol;
}

void ShutdownAudioSystem() {
    g_SFXRunning = false;
    g_SFXCond.notify_one();
    if (g_SFXThread.joinable()) {
        g_SFXThread.join();
    }

    StopBGM();
    mciSendStringA("close all", NULL, 0, NULL);
}