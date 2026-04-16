#pragma once
#include <string>

void PlayBGM(const std::string& filepath);
void StopBGM();
void UpdateBGMVolume();

void PlaySFX(const std::string& filepath, const std::string& alias);
void StopSFX(const std::string& alias);