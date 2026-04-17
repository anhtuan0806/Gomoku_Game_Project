#pragma once
#include <string>

// Nạp tất cả âm thanh vào bộ nhớ lúc mở game
void InitAudioSystem();

void PlayBGM(const std::string& filepath);
void StopBGM();
void UpdateBGMVolume();

// Chỉnh lại hàm PlaySFX chỉ nhận alias (vì đã nạp sẵn rồi)
void PlaySFX(const std::string& alias);
void StopSFX(const std::string& alias);

// Đóng toàn bộ âm thanh khi thoát game để tránh rò rỉ
void ShutdownAudioSystem();