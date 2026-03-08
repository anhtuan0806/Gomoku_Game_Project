#include "SaveLoadSystem.h"
#include <fstream>

bool SaveMatchData(const PlayState* state, const char* filepath) {
    // Dùng ios::binary để ghi toàn bộ khối nhớ của struct xuống file
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(state), sizeof(PlayState));
    file.close();
    return true;
}

bool LoadMatchData(PlayState* state, const char* filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.read(reinterpret_cast<char*>(state), sizeof(PlayState));
    file.close();
    return true;
}