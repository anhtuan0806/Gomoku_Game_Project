#include "Localization.h"
#include <fstream>
#include <map>

static std::map<std::string, std::string> g_dictionary;

void LoadLanguageFile(Language lang) {
    g_dictionary.clear();
    std::string filepath = (lang == APP_LANG_VIETNAMESE) ? "Asset/lang/vi.txt" : "Asset/lang/en.txt";
    std::ifstream file(filepath);

    if (!file.is_open()) {
        g_dictionary["missing_file"] = "Lỗi: Không tìm thấy tệp ngôn ngữ!";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            g_dictionary[key] = value;
        }
    }
    file.close();
}

std::string GetText(const std::string& key) {
    if (g_dictionary.find(key) != g_dictionary.end()) {
        return g_dictionary[key];
    }
    return key;
}