#include <iostream>
#include <string>

// --- Thư viện Đồ họa ---
#include <raylib.h>

// --- Include Kiểu dữ liệu ---
#include "ApplicationTypes/GameState.h"
#include "ApplicationTypes/GameConfig.h"
#include "ApplicationTypes/PlayState.h"

// --- Include Hệ thống (System) ---
#include "SystemModules/ConfigLoader.h"
#include "SystemModules/AudioSystem.h"
#include "SystemModules/Localization.h"
#include "SystemModules/TimeSystem.h"
#include "SystemModules/SaveLoadSystem.h"

// --- Include Đồ họa (Render API) ---
#include "RenderAPI/Renderer.h"
#include "RenderAPI/UIComponents.h"

// --- Include Các Màn hình (Screen Modules) ---
#include "ScreenModules/MenuScreen.h"
#include "ScreenModules/PlayScreen.h"
#include "ScreenModules/SettingScreen.h"
#include "ScreenModules/LoadGameScreen.h"
#include "GameLogic/StateUpdater.h"

int main() {
    // 1. KHỞI TẠO CỬA SỔ ĐỒ HỌA (Pixel: 1000x700)
    const int screenWidth = 1000;
    const int screenHeight = 700;
    InitGameWindow(screenWidth, screenHeight, "DO AN CARO & TIC-TAC-TOE");

    // 2. TẢI CẤU HÌNH VÀ KHỞI TẠO HỆ THỐNG
    GameConfig config;
    LoadConfig(&config, "Asset/config.ini");

    InitAudio(&config);
    LoadLanguageFile(config.currentLang);
    InitTimeSystem();

    // 3. KHAI BÁO BIẾN TRẠNG THÁI
    ScreenState currentState = SCREEN_MENU;
    PlayState playState;

    // Biến phụ cho Menu
    int menuSelection = 0;
    int settingSelection = 0;
    int loadSelection = 0;
    std::string loadStatusMsg = "";

    // Khởi tạo mặc định cho ván đấu mới
    InitNewMatch(&playState, MODE_CARO, MATCH_PVP, 12, 30);

    // Bắt đầu phát nhạc nền
    PlayBGM("Asset/audio/bgm_menu.wav");

    // 4. VÒNG LẶP GAME (GAME LOOP)
    while (IsWindowRunning() && currentState != SCREEN_EXIT) {

        // --- A. CẬP NHẬT LOGIC (UPDATE) ---
        double dt = GetDeltaTime(); // Lấy thời gian thực trôi qua [cite: 130]

        switch (currentState) {
        case SCREEN_MENU:
            UpdateMenuScreen(currentState, menuSelection);
            break;
        case SCREEN_PLAY:
            UpdatePlayScreen(&playState, currentState, dt);
            break;
        case SCREEN_SETTING:
            UpdateSettingScreen(currentState, &config, settingSelection);
            break;
        case SCREEN_LOAD_GAME:
            UpdateLoadGameScreen(currentState, &playState, loadSelection, loadStatusMsg);
            break;
        case SCREEN_ABOUT:
        case SCREEN_GUIDE:
            if (IsKeyPressed(KEY_ESCAPE)) currentState = SCREEN_MENU;
            break;
        default:
            break;
        }

        // --- B. VẼ GIAO DIỆN (RENDER) ---
        BeginRender();
        ClearScreenBackground(RAYWHITE); // Xóa màn hình bằng màu trắng mượt mà

        switch (currentState) {
        case SCREEN_MENU:
            RenderMenuScreen(menuSelection, screenWidth, screenHeight);
            break;
        case SCREEN_PLAY:
            RenderPlayScreen(&playState, screenWidth, screenHeight);
            break;
        case SCREEN_SETTING:
            RenderSettingScreen(&config, settingSelection, screenWidth, screenHeight);
            break;
        case SCREEN_LOAD_GAME:
            RenderLoadGameScreen(loadSelection, loadStatusMsg, screenWidth, screenHeight);
            break;
        case SCREEN_ABOUT:
            DrawTextCentered(screenHeight / 2, screenWidth, "DO AN CO CARO - CS161", 40, DARKGRAY);
            DrawTextCentered(screenHeight / 2 + 50, screenWidth, "Nhan ESC de quay lai", 20, GRAY);
            break;
        case SCREEN_GUIDE:
            DrawTextCentered(screenHeight / 2, screenWidth, "W,A,S,D: DI CHUYEN | ENTER: DANH CO", 30, DARKBLUE);
            DrawTextCentered(screenHeight / 2 + 50, screenWidth, "Nhan ESC de quay lai", 20, GRAY);
            break;
        default:
            break;
        }

        EndRender();
    }

    // 5. GIẢI PHÓNG TÀI NGUYÊN VÀ THOÁT 
    StopBGM();
    CloseGameWindow();
    SaveConfig(&config, "Asset/config.ini");

    return 0;
}