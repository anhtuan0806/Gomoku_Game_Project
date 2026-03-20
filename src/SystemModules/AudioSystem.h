#pragma once
#include <string>

void PlayBGM(const std::string& filepath);
void StopBGM();

// Dùng std::wstring cho hiệu ứng SFX vì hàm PlaySoundW yêu cầu Unicode
void PlaySFX(const std::wstring& filepath);