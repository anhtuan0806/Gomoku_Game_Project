#include "SaveLoadSystem.h"
#include "../ApplicationTypes/GameConstants.h"
#include <filesystem>
#include <fstream>
#include <windows.h>

bool SaveMatchData(const PlayState* state, const std::wstring& filename) {
    CreateDirectoryW(L"Asset", NULL);
    CreateDirectoryW(L"Asset/save", NULL);

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(state), sizeof(PlayState));

    file.close();
    return true;
}

bool LoadMatchData(PlayState* state, const std::wstring& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    file.read(reinterpret_cast<char*>(state), sizeof(PlayState));
    file.close();
    return true;
}

std::wstring GetSavePath(int slot) {
    return L"Asset/save/slot_" + std::to_wstring(slot) + L".bin";
}

bool CheckSaveExists(int slot) {
    DWORD dwAttrib = GetFileAttributesW(GetSavePath(slot).c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}