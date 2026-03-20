#ifndef LOAD_GAME_SCREEN_H
#define LOAD_GAME_SCREEN_H
#include "../ApplicationTypes/GameState.h"
#include "../ApplicationTypes/PlayState.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/SaveLoadSystem.h"
#include <string>
#include <windows.h>

/**
 * @brief Xử lý sự kiện bàn phím cho màn hình Tải Game.
 * @return bool Trả về true nếu cần vẽ lại màn hình 
 */
bool ProcessLoadGameInput(WPARAM wParam, ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage);

/**
 * @brief Kết xuất giao diện Màn hình Tải Game lên Back Buffer.
 */
void RenderLoadGameScreen(HDC hdc, int selectedOption, const std::wstring& statusMessage, int screenWidth, int screenHeight);

/**
 * @brief Nhận thông điệp từ WndProc và định tuyến xử lý logic đầu vào.
 */
void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage, WPARAM wParam);

#endif // LOAD_GAME_SCREEN_H 