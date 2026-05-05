#ifndef _CONFIGLOADER_H_
#define _CONFIGLOADER_H_
#include "../ApplicationTypes/GameConfig.h"
#include <string>

/** @file ConfigLoader.h
 *  @brief Tải và lưu cấu hình ứng dụng (như `GameConfig`).
 */

/**
 * @brief Tải cấu hình từ `filepath` vào `config`.
 * @details Nếu file không tồn tại hoặc không hợp lệ thì ghi giá trị mặc định vào `config`.
 * @param config Con trỏ tới `GameConfig` sẽ được điền.
 * @param filepath Đường dẫn file cấu hình (binary format).
 */
void LoadConfig(GameConfig *config, const std::string &filepath);

/**
 * @brief Lưu cấu hình hiện tại ra `filepath` theo dạng nhị phân.
 * @param config Tham chiếu tới cấu hình hiện hành.
 * @param filepath Đường dẫn file để ghi.
 */
void SaveConfig(const GameConfig *config, const std::string &filepath);

#endif // _CONFIGLOADER_H_