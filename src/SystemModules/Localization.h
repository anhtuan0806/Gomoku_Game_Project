#pragma once
#include "../ApplicationTypes/GameConfig.h"
#include <string>

// Tải ngôn ngữ vào bộ nhớ
void LoadLanguageFile(Language lang);

// Lấy chuỗi ký tự theo mã (Ví dụ: GetText("menu_start_game"))
std::string GetText(const std::string& key);