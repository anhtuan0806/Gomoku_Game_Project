#ifndef _AUDIOSYSTEM_H_
#define _AUDIOSYSTEM_H_
#include <string>

void SFXWorker(); // Hàm luồng nền xử lý SFX
void PreLoad(const std::string &filepath, const std::string &alias);

// Nạp tất cả âm thanh vào bộ nhớ lúc mở game
void InitAudioSystem();

void PlayBGM(const std::string &filepath);
void StopBGM();
void UpdateBGMVolume(bool force = false);

// hàm PlaySFX chỉ nhận alias (vì đã nạp sẵn rồi)
void PlaySFX(const std::string &alias);
void StopSFX(const std::string &alias);

// Đóng toàn bộ âm thanh khi thoát game để tránh rò rỉ
void ShutdownAudioSystem();

#endif // _AUDIOSYSTEM_H_