#pragma once
#include "../ApplicationTypes/PlayState.h"

// Lưu trạng thái ván đấu hiện tại vào tệp
bool SaveMatchData(const PlayState* state, const char* filepath);

// Tải trạng thái ván đấu từ tệp lên
bool LoadMatchData(PlayState* state, const char* filepath);