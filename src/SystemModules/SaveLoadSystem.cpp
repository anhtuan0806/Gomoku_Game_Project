#include "SaveLoadSystem.h"
#include <fstream>

bool SaveMatchData(const PlayState* state, const std::wstring& filename) {
    // Mở file ở chế độ nhị phân (Binary)
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Ghi toàn bộ khối nhớ của PlayState trực tiếp xuống ổ cứng
    file.write(reinterpret_cast<const char*>(state), sizeof(PlayState));
    file.close();
    return true;
}

bool LoadMatchData(PlayState* state, const std::wstring& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Đọc khối nhớ từ file và đắp thẳng vào struct PlayState
    file.read(reinterpret_cast<char*>(state), sizeof(PlayState));
    file.close();
    return true;
}