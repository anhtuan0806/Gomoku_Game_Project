#include "AudioSystem.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") // Liên kết thư viện âm thanh Windows

void PlayBGM(const std::string& filepath) {
    // Dừng nhạc cũ trước khi phát nhạc mới
    StopBGM();

    // Sử dụng mciSendString để có thể phát lặp lại (hỗ trợ .wav và .mp3)
    std::string cmd = "open \"" + filepath + "\" type mpegvideo alias bgm";
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
    mciSendStringA("play bgm repeat", NULL, 0, NULL);
}

void StopBGM() {
    mciSendStringA("close bgm", NULL, 0, NULL);
}

void PlaySFX(const std::wstring& filepath) {
    // Phát âm thanh ngắn (bắn cờ, click), cờ SND_ASYNC giúp game không bị khựng lại chờ âm thanh phát xong
    PlaySoundW(filepath.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}