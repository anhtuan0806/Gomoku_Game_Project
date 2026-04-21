#ifndef _LOCALIZATION_H_
#define _LOCALIZATION_H_
#include "../ApplicationTypes/GameConfig.h"
#include <string>

void LoadLanguageFile(Language lang);
std::wstring GetText(const std::string &key);

#endif // _LOCALIZATION_H_