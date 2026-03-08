#pragma once
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include <string>

// Xử lý phím điều hướng để chọn bản lưu bằng Raylib
// Nếu người chơi chọn Tải thành công, currentState sẽ chuyển thẳng sang SCREEN_PLAY
void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::string& statusMessage);

// Vẽ màn hình danh sách các bản lưu (Save Slots) bằng tọa độ pixel
void RenderLoadGameScreen(int selectedOption, const std::string& statusMessage, int screenWidth, int screenHeight);