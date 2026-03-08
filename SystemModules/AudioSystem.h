#pragma once
#include "../ApplicationTypes/GameConfig.h"

// Liên kết dữ liệu cài đặt âm thanh (Bật/tắt)
void InitAudio(const GameConfig* config);

// Phát nhạc nền lặp lại (file .wav)
void PlayBGM(const char* filepath);

// Dừng nhạc nền
void StopBGM();

// Phát âm thanh hiệu ứng 1 lần (file .wav)
void PlaySFX(const char* filepath);