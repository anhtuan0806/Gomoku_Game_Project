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

/** @file AudioSystem.cpp
 *  @brief Cài đặt hệ thống âm thanh: quản lý BGM, queue SFX và worker xử lý I/O.
 *
 *  Thiết kế:
 *  - SFX được preload và gán alias để phát nhanh bằng `mciSendString`.
 *  - Một worker thread tiêu thụ `g_SFXQueue` để thực hiện preload/play/stop không chặn UI.
 */

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

/**
 * @brief Mở một alias MCI cho file âm thanh tại `path`.
 * @param path Đường dẫn file âm thanh.
 * @param alias Khóa alias để tham chiếu sau này.
 * @return true nếu mở thành công.
 * @note Sử dụng `type mpegvideo` để có khả năng điều chỉnh volume tốt hơn trên một số hệ thống.
 */
static bool openSfxAlias(const std::string &path, const std::string &alias)
{
    std::string openCmd = "open \"" + path + "\" type mpegvideo alias " + alias;
    return mciSendStringA(openCmd.c_str(), NULL, 0, NULL) == 0;
}

/** @brief Khởi tạo worker thread xử lý SFX (đảm bảo chỉ chạy một lần). */
static void ensureSfxThread()
{
    std::call_once(g_SFXOnce, []()
                   {
        g_SFXRunning = true;
        g_SFXThread = std::thread(sfxWorker); });
}

/**
 * @brief Yêu cầu preload một file âm thanh: nếu worker chưa chạy thì thực hiện đồng bộ,
 *        ngược lại đẩy yêu cầu vào queue để worker xử lý.
 * @param path Đường dẫn file âm thanh.
 * @param alias Khóa alias để dùng khi phát.
 */
void preLoad(const std::string &path, const std::string &alias)
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

        g_SFXReady[alias] = openSfxAlias(path, alias);
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

/** @brief Khởi tạo hệ thống âm thanh: bật worker và preload các SFX cơ bản. */
void initAudioSystem()
{
    ensureSfxThread();
    preLoad("Asset/audio/move.wav", "sfx_move");
    preLoad("Asset/audio/select.wav", "sfx_select");
    preLoad("Asset/audio/DatCo.wav", "sfx_place");
    preLoad("Asset/audio/Tiengcoi.wav", "sfx_whistle");
    preLoad("Asset/audio/MatLuot.wav", "sfx_timeout");
    preLoad("Asset/audio/select.wav", "sfx_error");
    preLoad("Asset/audio/siuuu.wav", "sfx_success");
    preLoad("Asset/audio/siuuu.wav", "sfx_siu");
    preLoad("Asset/audio/TiengKhanGia_Het_Tran.wav", "sfx_crowd");
}

/** @brief Yêu cầu phát SFX theo `alias` (được enqueue để không block UI).
 *  @param alias Khóa SFX đã nạp.
 */
void playSfx(const std::string &alias)
{
    if (!g_Config.isSfxEnabled || g_Config.sfxVolume <= 0)
        return;
    ensureSfxThread();

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

/** @brief Yêu cầu dừng SFX theo `alias` (enqueue nếu worker hoạt động).
 *  @param alias Khóa SFX.
 */
void stopSfx(const std::string &alias)
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

/** @brief Phát BGM từ file `filepath` (lặp lại).
 *  @note Sử dụng MCI alias `bgm` cho nhạc nền.
 */
void playBgm(const std::string &filepath)
{
    if (!g_Config.isBgmEnabled)
        return;
    stopBgm();
    // BGM cũng dùng mpegvideo
    std::string cmd = "open \"" + filepath + "\" type mpegvideo alias bgm";
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
    updateBgmVolume(true);
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
}

/** @brief Dừng và đóng alias BGM nếu đang mở. */
void stopBgm()
{
    mciSendStringA("close bgm", NULL, 0, NULL);
}

/** @brief Cập nhật âm lượng BGM theo cấu hình `g_Config`.
 *  @param force Nếu true thì áp dụng ngay cả khi không đổi so với lần trước.
 */
void updateBgmVolume(bool force)
{
    static int lastBgmVol = -1;
    int vol = g_Config.bgmVolume * 10;
    if (!force && lastBgmVol == vol)
        return;

    std::string volCmd = "setaudio bgm volume to " + std::to_string(vol);
    mciSendStringA(volCmd.c_str(), NULL, 0, NULL);
    lastBgmVol = vol;
}

/** @brief Dừng worker, đóng các alias và giải phóng tài nguyên âm thanh. */
void shutdownAudioSystem()
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
    stopBgm();
    mciSendStringA("close all", NULL, 0, NULL);
}

/** @brief Luồng tiêu thụ `g_SFXQueue` để thực hiện preload/play/stop một cách tuần tự.
 *  Luồng này chạy cho tới khi `g_SFXRunning` = false và queue rỗng.
 */
void sfxWorker()
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

            bool opened = openSfxAlias(req.path, req.alias);
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

            bool opened = openSfxAlias(path, req.alias);
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
