#pragma once
#include "../ApplicationTypes/GameConfig.h"

// Đọc cài đặt từ tệp config.ini
void LoadConfig(GameConfig* config, const char* filepath);

// Ghi cài đặt xuống tệp config.ini
void SaveConfig(const GameConfig* config, const char* filepath);