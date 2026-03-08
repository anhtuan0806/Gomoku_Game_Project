#include "AudioSystem.h"
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib") // Lệnh này giúp VS2022 tự link thư viện âm thanh

static const GameConfig* g_audioConfig = nullptr;

void InitAudio(const GameConfig* config) {
    g_audioConfig = config;
}

void PlayBGM(const char* filepath) {
    if (g_audioConfig && g_audioConfig->isBgmEnabled) {
        PlaySoundA(filepath, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }
}

void StopBGM() {
    PlaySoundA(NULL, NULL, 0);
}

void PlaySFX(const char* filepath) {
    if (g_audioConfig && g_audioConfig->isSfxEnabled) {
        PlaySoundA(filepath, NULL, SND_FILENAME | SND_ASYNC);
    }
}