#include "Localization.h"
#include <fstream>
#include <map>
#include <windows.h>

/** @file Localization.cpp
 *  @brief Triển khai nạp file ngôn ngữ dạng key=value (UTF-8) và truy vấn chuỗi.
 */

static std::map<std::string, std::wstring> g_dictionary;

/** @brief Chuyển chuỗi UTF-8 sang std::wstring (UTF-16 trên Windows).
 *  @note Trả về chuỗi rỗng nếu đầu vào rỗng.
 */
static std::wstring UTF8ToWString(const std::string &str)
{
    if (str.empty())
        return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

/**
 * @brief Tải file ngôn ngữ (key=value UTF-8) vào bộ nhớ đệm `g_dictionary`.
 * @param lang Ngôn ngữ cần nạp (`APP_LANG_VI` hoặc `APP_LANG_EN`).
 */
void LoadLanguageFile(Language lang)
{
    g_dictionary.clear();
    std::string filepath = (lang == APP_LANG_VI) ? "Asset/lang/vi.txt" : "Asset/lang/en.txt";
    std::ifstream file(filepath);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line))
    {
        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            g_dictionary[key] = UTF8ToWString(val);
        }
    }
}

/**
 * @brief Trả về chuỗi đã dịch cho `key`.
 * @details Nếu `key` không tồn tại trong từ điển, trả về `key` chuyển sang wstring và cache nó.
 */
std::wstring GetText(const std::string &key)
{
    auto it = g_dictionary.find(key);
    if (it != g_dictionary.end())
        return it->second;

    std::wstring wkey = UTF8ToWString(key);
    g_dictionary[key] = wkey;
    return wkey;
}