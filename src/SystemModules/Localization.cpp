#include "Localization.h"
#include <fstream>
#include <map>
#include <windows.h>

static std::map<std::string, std::wstring> g_dictionary;

static std::wstring UTF8ToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void LoadLanguageFile(Language lang) {
    g_dictionary.clear();
    std::string filepath = (lang == APP_LANG_VI) ? "Asset/lang/vi.txt" : "Asset/lang/en.txt";
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            g_dictionary[key] = UTF8ToWString(val);
        }
    }
}

std::wstring GetText(const std::string& key) {
    auto it = g_dictionary.find(key);
    if (it != g_dictionary.end()) return it->second;

    // Cache missing keys (or those not in file) to avoid repeated UTF8ToWString in hot loops
    std::wstring wkey = UTF8ToWString(key);
    g_dictionary[key] = wkey;
    return wkey;
}