#pragma once
#include "../ApplicationTypes/PlayState.h"

// Khởi tạo dữ liệu gốc cho một ván đấu mới 
void InitNewMatch(PlayState* state, PlayMode mode, MatchType type, uint8_t boardSize, uint8_t countdown);

// Chuyển lượt chơi và reset thời gian đếm ngược
void SwitchTurn(PlayState* state);