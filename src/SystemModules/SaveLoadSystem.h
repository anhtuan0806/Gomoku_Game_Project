#pragma once
#include "../ApplicationTypes/PlayState.h"
#include <string>

// Lưu toàn bộ trạng thái ván đấu hiện tại xuống file
bool SaveMatchData(const PlayState* state, const std::wstring& filename);

// Tải dữ liệu ván đấu từ file lên bộ nhớ
bool LoadMatchData(PlayState* state, const std::wstring& filename);

std::wstring GetSavePath(int slot);

bool CheckSaveExists(int slot);

struct SaveMetadata {
    std::wstring name;
    std::wstring timestamp;
    int mode;       // 0: Caro, 1: TTT
    int type;       // 0: PvP, 1: PvE
    int p1Wins;
    int p2Wins;
    int difficulty;
    int boardSize;
    bool exists;
    uint32_t version;
};

// Các hàm bổ sung cho hệ thống quản lý slot mới
bool DeleteSave(int slot);
bool RenameSave(int slot, const std::wstring& newName);
std::wstring GetSaveDisplayName(int slot);
SaveMetadata GetSaveMetadata(int slot);