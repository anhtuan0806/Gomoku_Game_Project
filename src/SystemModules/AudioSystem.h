#ifndef _AUDIOSYSTEM_H_
#define _AUDIOSYSTEM_H_
#include <string>

void sfxWorker(); // Hàm luồng nền xử lý SFX
void preLoad(const std::string &filepath, const std::string &alias);

// Nạp tất cả âm thanh vào bộ nhớ lúc mở game
void initAudioSystem();

void playBgm(const std::string &filepath);
void stopBgm();
void updateBgmVolume(bool force = false);

// hàm playSfx chỉ nhận alias (vì đã nạp sẵn rồi)
void playSfx(const std::string &alias);
void stopSfx(const std::string &alias);

// Đóng toàn bộ âm thanh khi thoát game để tránh rò rỉ
void shutdownAudioSystem();

// --- Backwards-compatible wrappers (temporary) ---
// These call the new camelCase APIs so existing call sites continue to work
inline void InitAudioSystem() { initAudioSystem(); }
inline void PlayBGM(const std::string &filepath) { playBgm(filepath); }
inline void StopBGM() { stopBgm(); }
inline void UpdateBGMVolume(bool force = false) { updateBgmVolume(force); }
inline void PlaySFX(const std::string &alias) { playSfx(alias); }
inline void StopSFX(const std::string &alias) { stopSfx(alias); }
inline void PreLoad(const std::string &filepath, const std::string &alias) { preLoad(filepath, alias); }
inline void SFXWorker() { sfxWorker(); }
inline void ShutdownAudioSystem() { shutdownAudioSystem(); }

#endif // _AUDIOSYSTEM_H_