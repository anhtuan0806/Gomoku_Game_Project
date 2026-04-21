#ifndef _CONFIGLOADER_H_
#define _CONFIGLOADER_H_
#include "../ApplicationTypes/GameConfig.h"
#include <string>

// Tải cài đặt. Nếu file không tồn tại, tự động tạo giá trị mặc định.
void LoadConfig(GameConfig *config, const std::string &filepath);

// Lưu cài đặt hiện hành.
void SaveConfig(const GameConfig *config, const std::string &filepath);

#endif // _CONFIGLOADER_H_