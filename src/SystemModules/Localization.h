#ifndef _LOCALIZATION_H_
#define _LOCALIZATION_H_
#include "../ApplicationTypes/GameConfig.h"
#include <string>

/** @file Localization.h
 *  @brief Hệ thống đa ngôn ngữ: tải file ngôn ngữ và truy vấn chuỗi.
 */

/** @brief Nạp file ngôn ngữ tương ứng với `lang` (vi/en).
 *  @param lang Ngôn ngữ cần nạp.
 */
void LoadLanguageFile(Language lang);

/** @brief Lấy chuỗi đã dịch theo `key`.
 *  @param key Khóa chuỗi (ASCII/UTF8).
 *  @return Văn bản Unicode (wstring). Nếu không tồn tại, trả về `key` dưới dạng wstring.
 */
std::wstring GetText(const std::string &key);

#endif // _LOCALIZATION_H_