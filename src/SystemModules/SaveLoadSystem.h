#pragma once
#include "../ApplicationTypes/PlayState.h"
#include <string>

// Lưu toàn bộ trạng thái ván đấu hiện tại xuống file
bool SaveMatchData(const PlayState* state, const std::wstring& filename);

// Tải dữ liệu ván đấu từ file lên bộ nhớ
bool LoadMatchData(PlayState* state, const std::wstring& filename);

std::wstring GetSavePath(int slot);

bool CheckSaveExists(int slot);