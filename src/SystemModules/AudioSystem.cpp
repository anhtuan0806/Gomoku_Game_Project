#include "AudioSystem.h"
#include "../ApplicationTypes/GameConfig.h"
#include <windows.h>
#include <mmsystem.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#pragma comment(lib, "winmm.lib")

extern GameConfig g_Config;

enum class SFXCommand
{
    Preload,
    Play,
    Stop
};

struct SFXRequest
{
    SFXCommand command = SFXCommand::Play;
    std::string alias;
    std::string path;
    int volume = 0;
};

static std::mutex g_SFXMutex;
static std::condition_variable g_SFXCond;
static std::queue<SFXRequest> g_SFXQueue;
static std::unordered_map<std::string, ULONGLONG> g_LastSFXTime;
static std::unordered_map<std::string, int> g_LastSFXVolume;
static std::unordered_map<std::string, std::string> g_SFXPaths;
static std::unordered_map<std::string, bool> g_SFXReady;
static std::thread g_SFXThread;
static std::atomic<bool> g_SFXRunning(false);
static std::once_flag g_SFXOnce;

static bool OpenSFXAlias(const std::string &path, const std::string &alias)
{
    // Use mpegvideo for better volume control with wav
    std::string openCmd = "open \"" + path + "\" type mpegvideo alias " + alias;
    return mciSendStringA(openCmd.c_str(), NULL, 0, NULL) == 0;
}

static void EnsureSFXThread()
{
    std::call_once(g_SFXOnce, []()
                   {
        g_SFXRunning = true;
        g_SFXThread = std::thread(SFXWorker); });
}

void PreLoad(const std::string &path, const std::string &alias)
{
    {
        std::lock_guard<std::mutex> lock(g_SFXMutex);
        g_SFXPaths[alias] = path;
        g_SFXReady[alias] = false;
    }

    if (!g_SFXRunning)
    {
        // Đóng alias cũ nếu có để tránh lỗi mở lặp lại
        std::string closeCmd = "close " + alias;
        mciSendStringA(closeCmd.c_str(), NULL, 0, NULL);

        g_SFXReady[alias] = OpenSFXAlias(path, alias);
        return;
    }

    SFXRequest req;
    req.command = SFXCommand::Preload;
    req.alias = alias;
    req.path = path;

    {
        std::lock_guard<std::mutex> lock(g_SFXMutex);
        if (g_SFXQueue.size() > 32)
            return;
        g_SFXQueue.push(req);
    }
    g_SFXCond.notify_one();
}

void InitAudioSystem()
{
    EnsureSFXThread();
    PreLoad("Asset/audio/move.wav", "sfx_move");
    PreLoad("Asset/audio/select.wav", "sfx_select");
    PreLoad("Asset/audio/DatCo.wav", "sfx_place");
    PreLoad("Asset/audio/Tiengcoi.wav", "sfx_whistle");
    PreLoad("Asset/audio/MatLuot.wav", "sfx_timeout");
    PreLoad("Asset/audio/select.wav", "sfx_error");
    PreLoad("Asset/audio/siuuu.wav", "sfx_success");
    PreLoad("Asset/audio/siuuu.wav", "sfx_siu");
    PreLoad("Asset/audio/TiengKhanGia_Het_Tran.wav", "sfx_crowd");
}

void PlaySFX(const std::string &alias)
{
    if (!g_Config.isSfxEnabled || g_Config.sfxVolume <= 0)
        return;
    EnsureSFXThread();

    SFXRequest req;
    req.command = SFXCommand::Play;
    req.alias = alias;
    req.volume = g_Config.sfxVolume * 10;

    {
        std::lock_guard<std::mutex> lock(g_SFXMutex);
        if (g_SFXQueue.size() > 32)
            return;
        g_SFXQueue.push(req);
    }
    g_SFXCond.notify_one();
}

void StopSFX(const std::string &alias)
{
    if (!g_SFXRunning)
    {
        std::string cmd = "stop " + alias;
        mciSendStringA(cmd.c_str(), NULL, 0, NULL);
        return;
    }

    SFXRequest req;
    req.command = SFXCommand::Stop;
    req.alias = alias;

    {
        std::lock_guard<std::mutex> lock(g_SFXMutex);
        if (g_SFXQueue.size() > 32)
            return;
        g_SFXQueue.push(req);
    }
    g_SFXCond.notify_one();
}

void PlayBGM(const std::string &filepath)
{
    if (!g_Config.isBgmEnabled)
        return;
    StopBGM();
    // BGM cũng dùng mpegvideo
    std::string cmd = "open \"" + filepath + "\" type mpegvideo alias bgm";
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
    UpdateBGMVolume(true);
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
}

void StopBGM()
{
    mciSendStringA("close bgm", NULL, 0, NULL);
}

void UpdateBGMVolume(bool force)
{
    static int lastBgmVol = -1;
    int vol = g_Config.bgmVolume * 10;
    if (!force && lastBgmVol == vol)
        return;

    std::string volCmd = "setaudio bgm volume to " + std::to_string(vol);
    mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
    lastBgmVol = vol;
}

void ShutdownAudioSystem()
{
    if (g_SFXRunning)
    {
        g_SFXRunning = false;
        g_SFXCond.notify_all();
    }
    if (g_SFXThread.joinable())
    {
        g_SFXThread.join();
    }
    StopBGM();
    mciSendStringA("close all", NULL, 0, NULL);
}

void SFXWorker()
{
    while (true)
    {
        SFXRequest req;
        {
            std::unique_lock<std::mutex> lock(g_SFXMutex);
            g_SFXCond.wait(lock, []()
                           { return !g_SFXQueue.empty() || !g_SFXRunning; });
            if (!g_SFXRunning && g_SFXQueue.empty())
                break;
            if (g_SFXQueue.empty())
                continue;
            req = g_SFXQueue.front();
            g_SFXQueue.pop();
        }

        if (req.command == SFXCommand::Preload)
        {
            std::string closeCmd = "close " + req.alias;
            mciSendStringA(closeCmd.c_str(), NULL, 0, NULL);

            bool opened = OpenSFXAlias(req.path, req.alias);
            {
                std::lock_guard<std::mutex> lock(g_SFXMutex);
                g_SFXReady[req.alias] = opened;
            }
            continue;
        }

        if (req.command == SFXCommand::Stop)
        {
            std::string cmd = "stop " + req.alias;
            mciSendStringA(cmd.c_str(), NULL, 0, NULL);
            continue;
        }

        if (req.command != SFXCommand::Play)
            continue;

        bool isReady = false;
        std::string path;
        {
            std::lock_guard<std::mutex> lock(g_SFXMutex);
            auto readyIt = g_SFXReady.find(req.alias);
            isReady = (readyIt != g_SFXReady.end() && readyIt->second);
            auto pathIt = g_SFXPaths.find(req.alias);
            if (pathIt != g_SFXPaths.end())
            {
                path = pathIt->second;
            }
        }
        if (!isReady && !path.empty())
        {
            std::string closeCmd = "close " + req.alias;
            mciSendStringA(closeCmd.c_str(), NULL, 0, NULL);

            bool opened = OpenSFXAlias(path, req.alias);
            {
                std::lock_guard<std::mutex> lock(g_SFXMutex);
                g_SFXReady[req.alias] = opened;
            }
        }

        ULONGLONG now = GetTickCount64();
        auto lastTimeIt = g_LastSFXTime.find(req.alias);
        if (lastTimeIt != g_LastSFXTime.end() && (now - lastTimeIt->second) < 50ULL)
        {
            continue;
        }
        g_LastSFXTime[req.alias] = now;

        int lastVol = -1;
        auto lastVolIt = g_LastSFXVolume.find(req.alias);
        if (lastVolIt != g_LastSFXVolume.end())
        {
            lastVol = lastVolIt->second;
        }
        if (lastVol != req.volume)
        {
            std::string volCmd = "setaudio " + req.alias + " volume to " + std::to_string(req.volume);
            mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
            g_LastSFXVolume[req.alias] = req.volume;
        }

        std::string stopCmd = "stop " + req.alias;
        mciSendStringA(stopCmd.c_str(), NULL, 0, NULL);
        std::string playCmd = "play " + req.alias + " from 0";
        mciSendStringA(playCmd.c_str(), NULL, 0, NULL);
    }
}