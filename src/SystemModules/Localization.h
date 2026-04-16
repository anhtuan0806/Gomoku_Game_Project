#pragma once
#include "../ApplicationTypes/GameConfig.h"
#include <string>

void LoadLanguageFile(Language lang);
std::wstring GetText(const std::string& key);